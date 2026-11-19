#pragma once

#include "constants.h"
#include <cstdint>
#include <sys/types.h>

enum class HttpReaderErrorCode : std::uint8_t {
    OK,
    RECV_FAILED,
    HEADER_SIZE_TOO_LARGE,
    CONNECTION_CLOSED,
};

enum class HttpConnErrorCode : std::uint8_t {
    OK,
    CONNECTION_CLOSED,
};

class HttpReader {
    char buf[READ_CHUNK_SIZE];
    char *m_cursor;
    size_t m_valid_bytes;
    int m_socket_fd;

  public:
    HttpReader(int socket);

    HttpReaderErrorCode read_until_headers_done(std::string_view &view);

    // Moves any leftover data from previous reads to the start of the buffer so the buffer always
    // points to the start of the section we're reading
    void align_buffer();
};

class HttpConnection {
    int m_sock_fd;
    HttpReader reader;

  public:
    HttpConnection(int socket);

    HttpConnErrorCode receive();

    bool operator==(const HttpConnection &other) const;
    bool operator==(const int other) const;

    // ~TcpConnection();
};
