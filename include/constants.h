#pragma once

#include <array>
#include <cstdint>
#include <string_view>

inline constexpr size_t KB(size_t x) { return x * 1024; }
inline constexpr size_t MB(size_t x) { return x * 1024 * 1024; }

inline constexpr size_t CONNECTIONS_BACKLOG = 20;
inline constexpr size_t MAX_REQUEST_TARGET_SIZE = KB(2);
inline constexpr size_t MAX_HEADER_LINE_SIZE = KB(8);
inline constexpr size_t MAX_HEADER_SECTION_SIZE = KB(16);
inline constexpr size_t MAX_HEADER_COUNT = 64;
inline constexpr size_t READ_CHUNK_SIZE = KB(16);
inline constexpr size_t WRITE_CHUNK_SIZE = KB(16);
inline constexpr size_t MAX_REQUEST_BODY_SIZE = MB(10);

inline constexpr std::string_view HEADERS_SECTION_MARK = "\r\n\r\n";
inline constexpr std::string_view WHITESPACE = " \t\r\n\v\f";
inline constexpr size_t NO_POSITION = std::string_view::npos;

enum class HttpMethod : std::uint8_t { GET, HEAD, CONNECT, POST, PUT, DELETE, OPTIONS, NONE };
struct MethodObject {
    std::string_view name;
    HttpMethod method;
};

inline constexpr std::array<MethodObject, 7> HTTP_METHODS = {{
    {"GET", HttpMethod::GET},
    {"HEAD", HttpMethod::HEAD},
    {"CONNECT", HttpMethod::CONNECT},
    {"POST", HttpMethod::POST},
    {"PUT", HttpMethod::PUT},
    {"DELETE", HttpMethod::DELETE},
    {"OPTIONS", HttpMethod::OPTIONS},
}};

enum class HttpVersion : std::uint8_t { HTTP_1_0, HTTP_1_1, NONE };