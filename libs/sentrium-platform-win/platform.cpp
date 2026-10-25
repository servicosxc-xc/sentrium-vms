#include <sentrium/platform.hpp>
#include <ShlObj.h>
#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>

namespace sentrium::platform_win {
namespace fs = std::filesystem;
using core::Error;
using core::Failure;
Handle::~Handle() { if (*this) CloseHandle(value_); }
Handle::Handle(Handle&& other) noexcept : value_(std::exchange(other.value_, INVALID_HANDLE_VALUE)) {}
Handle& Handle::operator=(Handle&& other) noexcept {
    if (this != &other) { if (*this) CloseHandle(value_); value_ = std::exchange(other.value_, INVALID_HANDLE_VALUE); }
    return *this;
}
std::wstring run_id() {
    GUID id{};
    if (FAILED(CoCreateGuid(&id))) throw Failure(Error::resource, "Run identity unavailable");
    wchar_t text[40]{};
    if (StringFromGUID2(id, text, 40) == 0) throw Failure(Error::resource, "Run identity conversion failed");
    std::wstring result(text + 1, 36);
    std::transform(result.begin(), result.end(), result.begin(), [](wchar_t c) { return c >= L'A' && c <= L'F' ? static_cast<wchar_t>(c + 32) : c; });
    return result;
}
std::string ascii(const std::wstring& value) {
    std::string result;
    for (auto c : value) {
        if (c < 32 || c > 126) throw Failure(Error::configuration, "Expected ASCII identifier");
        result += static_cast<char>(c);
    }
    return result;
}
static bool same_path(std::wstring_view a, std::wstring_view b) {
    return CompareStringOrdinal(a.data(), static_cast<int>(a.size()), b.data(), static_cast<int>(b.size()), TRUE) == CSTR_EQUAL;
}
static void absolute_local(const fs::path& path) {
    const auto s = path.native();
    if (s.size() < 3 || s.size() > 240 || s[1] != L':' || s[2] != L'\\'
        || !((s[0] >= L'A' && s[0] <= L'Z') || (s[0] >= L'a' && s[0] <= L'z'))
        || s.find(L'/', 0) != std::wstring::npos || s.find(L':', 2) != std::wstring::npos)
        throw Failure(Error::filesystem, "Only canonical local drive paths are permitted");
    for (const auto& part : path.relative_path()) {
        const auto p = part.native();
        if (p.empty() || p == L"." || p == L".." || p.back() == L'.' || p.back() == L' ')
            throw Failure(Error::filesystem, "Noncanonical path component rejected");
    }
}
fs::path authorized_root() {
    PWSTR raw = nullptr;
    const auto hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DONT_VERIFY, nullptr, &raw);
    std::unique_ptr<wchar_t, decltype(&CoTaskMemFree)> holder(raw, &CoTaskMemFree);
    if (FAILED(hr) || !raw) throw Failure(Error::filesystem, "LocalAppData resolution failed");
    fs::path result = fs::path(raw) / L"Sentrium" / L"VMS" / L"dev" / L"Foundation-002";
    absolute_local(result);
    return result;
}
void validate_root_input(const fs::path& input) {
    absolute_local(input);
    if (!same_path(input.native(), authorized_root().native()))
        throw Failure(Error::filesystem, "DataRoot override rejected");
}
void validate_leaf(std::wstring_view name) {
    if (name.empty() || name.size() > 80 || name.front() == L'.' || name.back() == L'.')
        throw Failure(Error::filesystem, "Invalid child name");
    for (auto c : name) {
        if (!((c >= L'a' && c <= L'z') || (c >= L'0' && c <= L'9') || c == L'-' || c == L'.'))
            throw Failure(Error::filesystem, "Unsafe child name");
    }
    auto stem = name.substr(0, name.find(L'.'));
    if (stem == L"con" || stem == L"prn" || stem == L"aux" || stem == L"nul" || stem == L"clock$"
        || (stem.size() == 4 && (stem.substr(0,3) == L"com" || stem.substr(0,3) == L"lpt") && stem[3] >= L'0' && stem[3] <= L'9'))
        throw Failure(Error::filesystem, "Reserved child name");
}
static void verify_handle(HANDLE handle, const fs::path& path, bool directory) {
    FILE_ATTRIBUTE_TAG_INFO attrs{};
    BY_HANDLE_FILE_INFORMATION info{};
    if (!GetFileInformationByHandleEx(handle, FileAttributeTagInfo, &attrs, sizeof(attrs))
        || !GetFileInformationByHandle(handle, &info)) throw Failure(Error::filesystem, "Handle inspection failed");
    if ((attrs.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0
        || ((attrs.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) != directory
        || (!directory && info.nNumberOfLinks != 1)) throw Failure(Error::filesystem, "Reparse point, type or hard-link rejected");
    std::wstring final_path(32768, L'\0');
    const auto length = GetFinalPathNameByHandleW(handle, final_path.data(), static_cast<DWORD>(final_path.size()), FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    if (length == 0 || length >= final_path.size()) throw Failure(Error::filesystem, "Final path unavailable");
    final_path.resize(length);
    if (!same_path(final_path, L"\\\\?\\" + path.native())) throw Failure(Error::filesystem, "Final handle destination mismatch");
}
Handle Directory::checked_directory(const fs::path& path, bool create) {
    if (create && !CreateDirectoryW(path.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
        throw Failure(Error::filesystem, "Directory creation failed");
    Handle h(CreateFileW(path.c_str(), FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
                         nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, nullptr));
    if (!h) throw Failure(Error::filesystem, "Directory open failed");
    verify_handle(h.get(), path, true);
    return h;
}
Directory Directory::root() {
    Directory result;
    result.path_ = authorized_root();
    // Pin every ancestor without FILE_SHARE_DELETE. Never follow reparse points.
    const auto local = result.path_.parent_path().parent_path().parent_path().parent_path();
    fs::path cursor = result.path_.root_path();
    result.handles_.push_back(checked_directory(cursor, false));
    bool create = false;
    for (const auto& part : result.path_.relative_path()) {
        cursor /= part;
        result.handles_.push_back(checked_directory(cursor, create));
        if (same_path(cursor.native(), local.native())) create = true;
    }
    return result;
}
Directory Directory::child(std::wstring_view leaf) const {
    validate_leaf(leaf);
    Directory result;
    // Duplicate all pins, so a child remains safe even if its parent object dies.
    for (const auto& h : handles_) {
        HANDLE duplicate = INVALID_HANDLE_VALUE;
        if (!DuplicateHandle(GetCurrentProcess(), h.get(), GetCurrentProcess(), &duplicate, 0, FALSE, DUPLICATE_SAME_ACCESS))
            throw Failure(Error::resource, "Directory handle duplication failed");
        result.handles_.emplace_back(duplicate);
    }
    result.path_ = path_ / leaf;
    result.handles_.push_back(checked_directory(result.path_, true));
    return result;
}
Handle Directory::file(std::wstring_view leaf, bool exclusive) const {
    validate_leaf(leaf);
    const auto path = path_ / leaf;
    Handle h(CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, exclusive ? 0 : FILE_SHARE_READ,
                        nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr));
    if (!h) throw Failure(Error::filesystem, "File unavailable or another run holds the lock");
    verify_handle(h.get(), path, false); // no truncation or write before this check
    return h;
}
void truncate_file(HANDLE file) {
    LARGE_INTEGER zero{};
    if (!SetFilePointerEx(file, zero, nullptr, FILE_BEGIN) || !SetEndOfFile(file))
        throw Failure(Error::filesystem, "Log truncation failed");
}
void write_file(HANDLE file, std::string_view text) {
    if (text.size() > MAXDWORD) throw Failure(Error::filesystem, "Write too large");
    DWORD written = 0;
    if (!WriteFile(file, text.data(), static_cast<DWORD>(text.size()), &written, nullptr) || written != text.size())
        throw Failure(Error::filesystem, "Log write failed");
}
FileLog::FileLog(const Directory& dir, core::Config config) : directory_(dir), config_(config) { config_.validate(); open_slot(); }
void FileLog::open_slot() {
    file_ = {};
    file_ = directory_.file(L"sentrium-" + std::to_wstring(slot_) + L".jsonl");
    truncate_file(file_.get());
    bytes_ = 0;
}
void FileLog::write(std::string_view record) {
    if (finished_) throw Failure(Error::internal, "Logging already finalized");
    if (record.size() > config_.log_bytes) throw Failure(Error::configuration, "Log record exceeds configured bound");
    if (bytes_ + record.size() > config_.log_bytes) {
        if (!FlushFileBuffers(file_.get())) throw Failure(Error::filesystem, "Log flush failed");
        slot_ = (slot_ + 1) % config_.log_files;
        open_slot();
    }
    write_file(file_.get(), record);
    bytes_ += record.size();
}
void FileLog::finish() {
    if (!finished_) {
        if (!FlushFileBuffers(file_.get())) throw Failure(Error::filesystem, "Log flush failed");
        file_ = {}; finished_ = true;
    }
}
int run(int argc, wchar_t** argv) {
    core::SystemClock clock;
    core::Lifecycle lifecycle(clock);
    std::string run = "unavailable";
    auto console_event = [&](std::string_view event, std::string_view message) {
        std::cerr << core::event_json(clock, "Info", event, "sentrium-server", message, run);
    };
    try {
        run = ascii(run_id());
        console_event("bootstrap-started", "Bootstrap started");
        core::Config config;
        bool health = false;
        std::wstring test_run;
        for (int i = 1; i < argc; ++i) {
            const std::wstring_view arg(argv[i]);
            if (arg == L"--health" && !health) health = true;
            else if (arg == L"--test-run" && test_run.empty() && i + 1 < argc) {
                test_run = argv[++i];
                validate_leaf(test_run);
                if (test_run.size() != 36 || test_run[8] != L'-' || test_run[13] != L'-' || test_run[18] != L'-' || test_run[23] != L'-')
                    throw Failure(Error::configuration, "Invalid isolated test identity");
            }
            else if (arg == L"--version" && argc == 2) {
                std::cout << core::product << ' ' << core::version << ' ' << core::build_id << ' ' << core::build_configuration << ' ' << core::build_compiler << '\n';
                return 0;
            } else throw Failure(Error::configuration, "Supported commands: --health or --version");
        }
        if (!health) throw Failure(Error::configuration, "Use --health for the Foundation-002 local diagnostic");
        config.validate();
        console_event("configuration-validated", "Configuration validated");
        auto root = Directory::root();
        auto runtime = test_run.empty() ? root.child(L"runtime") : root.child(L"tests").child(test_run).child(L"runtime");
        auto lock = runtime.file(L"sentrium.lock", true);
        auto logs = runtime.child(L"logs");
        FileLog log(logs, config);
        auto event = [&](std::string_view id, std::string_view message) { log.write(core::event_json(clock, "Info", id, "sentrium-server", message, run)); };
        event("bootstrap-started", "Bootstrap started");
        event("configuration-validated", "Configuration validated");
        event("dataroot-validated", "DataRoot validated by handle");
        auto activity = std::make_unique<core::Activity>();
        lifecycle.ready();
        auto diagnosis = lifecycle.health();
        event("health-calculated", "Local diagnostic ready");
        lifecycle.stopping(); // admission closed before cancellation
        event("shutdown-started", "Shutdown started; new activities blocked");
        event("cancellation-requested", "Cooperative cancellation requested");
        activity->stop();
        if (!activity->completed()) throw Failure(Error::resource, "Cancellation was not observed");
        activity.reset();
        lifecycle.stopped();
        event("resources-released", "Controlled activity joined and released");
        event("shutdown-completed", "Shutdown completed");
        log.finish();
        const auto final = lifecycle.health();
        diagnosis.telemetry.shutdown_ms = final.telemetry.shutdown_ms;
        diagnosis.telemetry.workers = final.telemetry.workers;
        std::cout << core::health_json(diagnosis);
        return diagnosis.ready() ? 0 : static_cast<int>(Error::resource);
    } catch (const Failure& e) {
        lifecycle.fail(e.code());
        console_event(e.code() == Error::configuration ? "configuration-rejected" : "dataroot-or-resource-rejected", e.what());
        console_event("fatal", "Local diagnostic failed; owned resources unwound");
        std::cout << core::health_json(lifecycle.health());
        return static_cast<int>(e.code());
    } catch (const std::exception&) {
        lifecycle.fail(Error::internal);
        console_event("fatal", "Unexpected bootstrap failure; owned resources unwound");
        std::cout << core::health_json(lifecycle.health());
        return static_cast<int>(Error::internal);
    }
}
}
