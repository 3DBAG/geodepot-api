#pragma once
#include <filesystem>
#include <stdexcept>
#include <string>
namespace geodepot {
struct PrepareOptions { std::filesystem::path lock_file; std::filesystem::path cache_root; std::string suite; bool offline = false; };
enum class ErrorCode { cli=2, missing=3, network=4, integrity=5, metadata=6, cache=7 };
class Error final : public std::runtime_error { public: Error(ErrorCode c, std::string m): std::runtime_error(std::move(m)), code_(c) {} [[nodiscard]] ErrorCode code() const noexcept { return code_; } private: ErrorCode code_; };
std::filesystem::path prepare(const PrepareOptions& options);
}
