#pragma once

#include "state.h"
#include <cstdint>
#include <string_view>
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
    HttpReader();
    void reset(int socket);

    HttpReader(const HttpReader &other) = delete;
    HttpReader(HttpReader &&other) = delete;
    HttpReader &operator=(const HttpReader &) = delete;
    HttpReader &operator=(HttpReader &&) = delete;

    HttpReaderErrorCode read_until_headers_done(std::string_view &view);

    // Moves any leftover data from previous reads to the start of the buffer so the buffer always
    // points to the start of the section we're reading
    void align_buffer();

    ~HttpReader();
};

class HttpWriter {
    char buf[WRITE_CHUNK_SIZE];
    int m_file_fd;
    int m_socket_fd;

  public:
    HttpWriter();
    void reset(int socket);

    HttpWriter(const HttpWriter &other) = delete;
    HttpWriter(HttpWriter &&other) = delete;
    HttpWriter &operator=(const HttpWriter &) = delete;
    HttpWriter &operator=(HttpWriter &&) = delete;

    int open_file(std::string_view file_path);

    ~HttpWriter();
};

class HttpConnection {
    HttpReader reader;
    HttpWriter writer;
    HttpState state;
    int m_socket_fd;

  public:
    HttpConnection();

    HttpConnection(const HttpConnection &other) = delete;
    HttpConnection(HttpConnection &&other) = delete;
    HttpConnection &operator=(const HttpConnection &) = delete;
    HttpConnection &operator=(HttpConnection &&) = delete;

    HttpConnErrorCode receive();
    void reset(int socket);

    bool operator==(const HttpConnection &other) const;
    bool operator==(const int other) const;

    ~HttpConnection();
};
