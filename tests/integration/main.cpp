#include <sentrium/platform.hpp>
#include <winioctl.h>
#include <TlHelp32.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>

using namespace sentrium;
namespace fs = std::filesystem;
using platform_win::Directory;
using platform_win::Handle;
static void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
template<class F> static void rejected(F action) {
    try { action(); } catch (const core::Failure&) { return; }
    throw std::runtime_error("unsafe input was accepted");
}
static std::string read(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    require(stream.good(), "read fixture");
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}
// All fixture directories are generated beneath the exclusive Foundation root.
// Cleanup does not follow symlinks/junctions; each reparse fixture is explicitly
// removed before this destructor. No operational paths enter this harness.
struct Fixture {
    std::wstring id = platform_win::run_id();
    fs::path path = platform_win::authorized_root() / L"tests" / id;
    ~Fixture() {
        std::error_code ec;
        if (path.parent_path() == platform_win::authorized_root() / L"tests") fs::remove_all(path, ec);
    }
    Directory open() const { return Directory::root().child(L"tests").child(id); }
};
struct Child { DWORD code; std::string output; std::string errors; DWORD pid; };
static Child execute(const fs::path& executable, const Fixture& fixture, std::wstring arguments) {
    auto dir = fixture.open();
    auto out = dir.file(L"stdout.txt"); auto err = dir.file(L"stderr.txt");
    platform_win::truncate_file(out.get()); platform_win::truncate_file(err.get());
    require(SetHandleInformation(out.get(), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT) != 0, "stdout inheritance");
    require(SetHandleInformation(err.get(), HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT) != 0, "stderr inheritance");
    STARTUPINFOW startup{}; startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = out.get(); startup.hStdError = err.get(); startup.hStdInput = nullptr;
    PROCESS_INFORMATION process{};
    auto command = L"\"" + executable.native() + L"\" " + arguments + L" --test-run " + fixture.id;
    require(CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process) != 0, "child launch");
    Handle proc(process.hProcess), thread(process.hThread);
    SetHandleInformation(out.get(), HANDLE_FLAG_INHERIT, 0); SetHandleInformation(err.get(), HANDLE_FLAG_INHERIT, 0);
    const auto wait = WaitForSingleObject(proc.get(), 15000);
    if (wait != WAIT_OBJECT_0) { TerminateProcess(proc.get(), 99); WaitForSingleObject(proc.get(), 5000); throw std::runtime_error("child timeout: forced cleanup, TEST FAIL"); }
    DWORD code = 99; require(GetExitCodeProcess(proc.get(), &code) != 0 && code != STILL_ACTIVE, "process completed");
    out = {}; err = {};
    return {code, read(dir.path()/L"stdout.txt"), read(dir.path()/L"stderr.txt"), process.dwProcessId};
}
static void mount_point(const fs::path& link, const fs::path& target) {
    require(CreateDirectoryW(link.c_str(), nullptr) != 0, "junction fixture directory");
    Handle h(CreateFileW(link.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, nullptr));
    require(static_cast<bool>(h), "junction fixture handle");
    const auto substitute = L"\\??\\" + target.native();
    const auto print = target.native();
    struct Mount { DWORD tag; WORD length; WORD reserved; WORD sub_offset; WORD sub_length; WORD print_offset; WORD print_length; wchar_t paths[1024]; } data{};
    data.tag = IO_REPARSE_TAG_MOUNT_POINT;
    data.sub_length = static_cast<WORD>(substitute.size()*sizeof(wchar_t));
    data.print_offset = static_cast<WORD>(data.sub_length + sizeof(wchar_t));
    data.print_length = static_cast<WORD>(print.size()*sizeof(wchar_t));
    std::copy(substitute.begin(), substitute.end(), data.paths);
    std::copy(print.begin(), print.end(), data.paths + substitute.size()+1);
    data.length = static_cast<WORD>(8 + data.print_offset + data.print_length + sizeof(wchar_t));
    DWORD returned = 0;
    require(DeviceIoControl(h.get(), FSCTL_SET_REPARSE_POINT, &data, static_cast<DWORD>(data.length)+8, nullptr, 0, &returned, nullptr) != 0, "create controlled junction");
}
static std::set<std::string> imports(const fs::path& exe) {
    const auto bytes = read(exe);
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(bytes.data());
    require(bytes.size() > sizeof(IMAGE_DOS_HEADER) && dos->e_magic == IMAGE_DOS_SIGNATURE, "PE DOS header");
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(bytes.data()+dos->e_lfanew);
    require(nt->Signature == IMAGE_NT_SIGNATURE && nt->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64, "PE AMD64");
    const auto* sections = IMAGE_FIRST_SECTION(nt);
    auto offset = [&](DWORD rva) -> std::size_t {
        for (WORD i=0; i<nt->FileHeader.NumberOfSections; ++i) {
            const auto& s = sections[i];
            if (rva >= s.VirtualAddress && rva < s.VirtualAddress+s.SizeOfRawData) return static_cast<std::size_t>(rva-s.VirtualAddress+s.PointerToRawData);
        }
        throw std::runtime_error("invalid import RVA");
    };
    auto pos = offset(nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    std::set<std::string> result;
    while (pos + sizeof(IMAGE_IMPORT_DESCRIPTOR) <= bytes.size()) {
        IMAGE_IMPORT_DESCRIPTOR item{}; std::memcpy(&item, bytes.data()+pos, sizeof(item));
        if (!item.Name) break;
        auto name_pos = offset(item.Name);
        auto end = bytes.find('\0', name_pos); require(end != std::string::npos, "import termination");
        auto dll = bytes.substr(name_pos, end-name_pos);
        std::transform(dll.begin(), dll.end(), dll.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        result.insert(dll); pos += sizeof(item);
    }
    return result;
}
int wmain(int argc, wchar_t** argv) {
    try {
        require(argc == 3, "id and executable required");
        const auto id = std::stoi(argv[1]);
        const fs::path executable(argv[2]);
        Fixture fixture;
        auto directory = fixture.open();
        switch (id) {
        case 1: {
            auto child = execute(executable,fixture,L"--health");
            require(child.code == 0 && child.output.find("\"State\":\"Ready\"") != std::string::npos, "bootstrap/shutdown");
            const auto log = read(directory.path()/L"runtime/logs/sentrium-0.jsonl");
            std::size_t previous = 0;
            for (auto event : {"bootstrap-started","configuration-validated","dataroot-validated","health-calculated","shutdown-started","cancellation-requested","resources-released","shutdown-completed"}) {
                auto position = log.find(event,previous); require(position != std::string::npos,"ordered shutdown log"); previous = position+1;
            }
            std::cout << child.output;
            break;
        }
        case 2: {
            auto child = execute(executable,fixture,L"--health --data-root synthetic-forbidden");
            require(child.code == 2 && !fs::exists(directory.path()/L"runtime"), "invalid configuration has no runtime effects");
            require(child.errors.find("configuration-rejected") != std::string::npos, "configuration error record");
            break;
        }
        case 3:
            platform_win::validate_root_input(platform_win::authorized_root());
            for (const auto& path : {fs::path(L"relative"),fs::path(L"\\\\synthetic-host\\share"),fs::path(L"\\\\?\\C:\\synthetic"),directory.path()/L"..",platform_win::authorized_root().parent_path()/L"Foundation-001",fs::path(L"C:\\sentrium-synthetic-forbidden")})
                rejected([&] { platform_win::validate_root_input(path); });
            for (auto leaf : {L"..",L"../escape",L"a:b",L"x\\y",L"con",L"lpt1.txt",L"trailing.",L" space",L"a/b",L"\\\\?\\C:"})
                rejected([&] { auto unused = directory.file(leaf); });
            break;
        case 4: {
            auto target = directory.child(L"target");
            auto marker = target.file(L"marker.txt"); platform_win::write_file(marker.get(),"unchanged"); marker = {};
            mount_point(directory.path()/L"junction",target.path());
            rejected([&] { auto unsafe = directory.child(L"junction"); });
            require(RemoveDirectoryW((directory.path()/L"junction").c_str()) != 0, "remove junction itself");
            const auto symlink = directory.path()/L"symlink";
            if (CreateSymbolicLinkW(symlink.c_str(),target.path().c_str(),SYMBOLIC_LINK_FLAG_DIRECTORY | SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE)) {
                rejected([&] { auto unsafe = directory.child(L"symlink"); });
                require(RemoveDirectoryW(symlink.c_str()) != 0, "remove symlink itself");
                std::cout << "Symlink fixture: verified\n";
            } else {
                // A symlink reparse buffer can be installed in the controlled fixture
                // only if Windows grants the capability. Never enable privileges.
                throw std::runtime_error("Symlink fixture unavailable; I-4 NOT VERIFIED");
            }
            const auto hard = directory.path()/L"hard.txt";
            require(CreateHardLinkW(hard.c_str(),(target.path()/L"marker.txt").c_str(),nullptr) != 0,"hard-link fixture");
            rejected([&] { auto unsafe = directory.file(L"hard.txt"); });
            require(DeleteFileW(hard.c_str()) != 0,"remove hard link");
            require(read(target.path()/L"marker.txt") == "unchanged","no writes through aliases");
            break;
        }
        case 5:
            for (int i=0;i<20;++i) { core::Activity work; work.stop(); require(work.completed(), "cooperative worker completion"); }
            break;
        case 6: {
            DWORD before = 0, after = 0;
            require(GetProcessHandleCount(GetCurrentProcess(),&before) != 0, "handle baseline");
            for (int i=0;i<20;++i) {
                auto file = directory.file(L"lifetime.txt"); auto moved = std::move(file);
                require(!file && static_cast<bool>(moved), "moved handle invalidated");
                core::Activity work;
            }
            require(GetProcessHandleCount(GetCurrentProcess(),&after) != 0 && before == after, "no leaked handles");
            auto child = execute(executable,fixture,L"--health"); require(child.code == 0,"child success");
            Handle process(OpenProcess(SYNCHRONIZE, FALSE, child.pid));
            require(!process || WaitForSingleObject(process.get(),0) == WAIT_OBJECT_0,"no orphan process");
            std::cout << "Handle count before=" << before << " after=" << after << '\n';
            break;
        }
        case 7: {
            auto logs = directory.child(L"bounded-logs");
            core::SystemClock clock;
            platform_win::FileLog log(logs,{1024,2});
            for (int i=0;i<100;++i) log.write(core::event_json(clock,"Info","rotation","sentrium-integration-tests","bounded record",platform_win::ascii(fixture.id)));
            log.finish();
            std::size_t count=0;
            for (const auto& file : fs::directory_iterator(logs.path())) { require(file.file_size() <= 1024,"log bound"); ++count; }
            require(count == 2,"rotation file count");
            rejected([&] { log.write("after finish"); });
            break;
        }
        case 8: {
            const auto first = execute(executable,fixture,L"--health");
            const auto second = execute(executable,fixture,L"--health");
            require(first.code == 0 && second.code == 0,"repeat diagnostic");
            for (auto field : {"\"State\":\"Ready\"","\"DataRootReady\":true","\"ResourcesReady\":true","\"Workers\":0","\"BootstrapFailures\":0","\"LastError\":null"})
                require(first.output.find(field) != std::string::npos && second.output.find(field) != std::string::npos,"reproducible logical field");
            break;
        }
        case 9: {
            const auto dlls = imports(executable);
            const std::set<std::string> allowed{"kernel32.dll","shell32.dll","ole32.dll","advapi32.dll","msvcp140.dll","msvcp140d.dll","msvcp140_atomic_wait.dll","msvcp140d_atomic_wait.dll","vcruntime140.dll","vcruntime140d.dll","vcruntime140_1.dll","vcruntime140_1d.dll","ucrtbased.dll","ucrtbase.dll","clang_rt.asan_dynamic-x86_64.dll"};
            require(!dlls.empty(),"PE imports present");
            for (const auto& dll : dlls) { std::cout << dll << '\n'; require(allowed.contains(dll) || dll.starts_with("api-ms-win-crt-"),"unauthorized binary dependency"); }
            break;
        }
        case 10: {
            const auto ok = execute(executable,fixture,L"--health"); require(ok.code == 0,"health exit");
            const auto bad = execute(executable,fixture,L"--unknown"); require(bad.code == 2,"invalid command exit");
            require(ok.output.find("C:\\") == std::string::npos && ok.output.find("RunId") == std::string::npos,"health privacy");
            auto runtime = directory.child(L"runtime"); auto lock = runtime.file(L"sentrium.lock",true);
            const auto busy = execute(executable,fixture,L"--health"); require(busy.code == 3,"exclusive runtime lock");
            break;
        }
        case 11: {
            const auto child = execute(executable,fixture,L"--health");
            require(child.code == 0 && child.output.find(core::version) != std::string::npos && child.output.find(core::build_id) != std::string::npos,"health build metadata");
            const auto log = read(directory.path()/L"runtime/logs/sentrium-0.jsonl");
            require(log.find(core::version) != std::string::npos && log.find(core::build_id) != std::string::npos && log.find(platform_win::ascii(fixture.id)) == std::string::npos,"log metadata and separate RunId");
            break;
        }
        default: throw std::runtime_error("unknown test");
        }
        std::cout << "PASS I-" << id << ' ' << core::version << ' ' << core::build_id << '\n';
        return 0;
    } catch (const std::exception& e) { std::cerr << "FAIL " << e.what() << '\n'; return 1; }
}
