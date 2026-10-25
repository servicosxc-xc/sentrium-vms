#pragma once
#include <sentrium/core.hpp>
#include <Windows.h>
#include <filesystem>
#include <vector>

namespace sentrium::platform_win {
class Handle final {
public:
    Handle() noexcept = default;
    explicit Handle(HANDLE value) noexcept : value_(value) {}
    ~Handle();
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    Handle(Handle&& other) noexcept;
    Handle& operator=(Handle&& other) noexcept;
    HANDLE get() const noexcept { return value_; }
    explicit operator bool() const noexcept { return value_ != INVALID_HANDLE_VALUE && value_ != nullptr; }
private:
    HANDLE value_ = INVALID_HANDLE_VALUE;
};
std::wstring run_id();
std::string ascii(const std::wstring& value);
std::filesystem::path authorized_root();
void validate_root_input(const std::filesystem::path& input);
void validate_leaf(std::wstring_view name);
class Directory final {
public:
    static Directory root();
    Directory child(std::wstring_view leaf) const;
    Handle file(std::wstring_view leaf, bool exclusive = false) const;
    const std::filesystem::path& path() const noexcept { return path_; }
    HANDLE handle() const noexcept { return handles_.back().get(); }
private:
    std::filesystem::path path_;
    std::vector<Handle> handles_;
    static Handle checked_directory(const std::filesystem::path& path, bool create);
};
void truncate_file(HANDLE file);
void write_file(HANDLE file, std::string_view text);
class FileLog final : public core::LogSink {
public:
    FileLog(const Directory& directory, core::Config config);
    void write(std::string_view record) override;
    void finish() override;
private:
    const Directory& directory_;
    core::Config config_;
    std::size_t slot_ = 0;
    std::size_t bytes_ = 0;
    Handle file_;
    bool finished_ = false;
    void open_slot();
};
int run(int argc, wchar_t** argv);
}
