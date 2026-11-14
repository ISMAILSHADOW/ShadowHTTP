#pragma once

#include <string_view>
inline constexpr size_t KB(size_t x) { return x * 1024; }
inline constexpr size_t MB(size_t x) { return x * 1024 * 1024; }

inline constexpr size_t CONNECTIONS_BACKLOG = 20;
inline constexpr size_t MAX_HEADER_LINE_SIZE = KB(8);
inline constexpr size_t MAX_HEADER_SECTION_SIZE = KB(16);
inline constexpr size_t READ_CHUNK_SIZE = KB(16);

inline constexpr std::string_view HEADERS_SECTION_MARK = "\r\n\r\n";
