#include "connection.h"
#include "constants.h"
#include "log.h"
#include "parser.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>

HttpConnection::HttpConnection() : m_socket_fd{-1} {}

HttpConnErrorCode HttpConnection::receive() {
    std::string_view view;
    if (auto ret = reader.read_until_headers_done(view); ret != HttpReaderErrorCode::OK) {
        std::cerr << "TcpConnection::receive() Error " << (int)ret << std::endl;
        return HttpConnErrorCode::CONNECTION_CLOSED;
    }

    HttpParser parser(view);
    if (auto ret = parser.parse_request_line(state); ret != HttpParserErrorCode::OK) {
        std::cerr << "Failed to parse request line: " << (int)ret << std::endl;
        return HttpConnErrorCode::CONNECTION_CLOSED;
    }

    if (auto ret = parser.parse_headers(state); ret != HttpParserErrorCode::OK) {
        std::cerr << "Failed to parse request headers: " << (int)ret << std::endl;
        return HttpConnErrorCode::CONNECTION_CLOSED;
    }

    std::cout << "Path: " << state.uri.path() << std::endl;

    if (writer.open_file(state.uri.path())) {
    }

    std::cout << "Received " << view.size() << " Bytes" << std::endl;
    std::cout << view << std::endl;

    std::string body = "Hello, world.";
    std::string buf =
        "HTTP/1.1 200 OK\r\nServer: Apache/2.4.41 (Ubuntu)\r\nContent-Length: " + std::to_string(body.size()) +
        "\r\nConnection: keep-alive\r\n\r\n" + body;
    send(m_socket_fd, buf.data(), buf.size(), 0);
    return HttpConnErrorCode::OK;
}

bool HttpConnection::operator==(const HttpConnection &other) const { return m_socket_fd == other.m_socket_fd; }
bool HttpConnection::operator==(const int other) const { return m_socket_fd == other; }

void HttpConnection::reset(int socket) {
    if (m_socket_fd != -1) close(socket);
    m_socket_fd = socket;
    reader.reset(socket);
    writer.reset(socket);
    state.reset();
}

HttpConnection::~HttpConnection() {
    if (m_socket_fd != -1) close(m_socket_fd);
}

HttpReader::HttpReader() : m_cursor(buf), m_valid_bytes(0) {}

void HttpReader::align_buffer() {
    // Just being paranoid to catch bugs early.
    assert(m_cursor >= buf && m_cursor <= buf + READ_CHUNK_SIZE);
    assert(m_cursor + m_valid_bytes <= buf + READ_CHUNK_SIZE);

    if (m_cursor != buf) {
        memmove(buf, m_cursor, m_valid_bytes);
        m_cursor = buf;
    }
}

HttpReaderErrorCode HttpReader::read_until_headers_done(std::string_view &view) {
    align_buffer();

    do {
        ssize_t bytes = recv(m_socket_fd, buf + m_valid_bytes, READ_CHUNK_SIZE - (size_t)m_valid_bytes, 0);
        if (bytes <= 0) {
            m_valid_bytes = 0;
            m_cursor = buf;
            if (bytes == -1) return HttpReaderErrorCode::RECV_FAILED;
            return HttpReaderErrorCode::CONNECTION_CLOSED;
        }

        m_valid_bytes += (size_t)bytes;
        view = std::string_view(buf, m_valid_bytes);
        size_t pos = view.find(HEADERS_SECTION_MARK);

        if (pos != NO_POSITION) { // If found
            pos += 4;             // To get end of headers section.
            m_valid_bytes -= pos;
            m_cursor = buf + pos;                     // To remember to shift on subsequent calls
            view = std::string_view(buf, pos);        // This is to include only the header section
            assert(m_valid_bytes <= READ_CHUNK_SIZE); // Checking for underflows
            break;
        }

        if (m_valid_bytes == READ_CHUNK_SIZE) {
            LOG_DEBUG("max header size exceeded with not \\r\\n\\r\\r");
            m_valid_bytes = 0;
            m_cursor = buf;
            return HttpReaderErrorCode::HEADER_SIZE_TOO_LARGE;
        }
    } while (true); // TODO change the while loop to use epoll or similar mechanisms when you
                    // learn that later

    return HttpReaderErrorCode::OK;
}

void HttpReader::reset(int socket) {
    m_socket_fd = socket;
    m_cursor = buf;
    m_valid_bytes = 0;
}

HttpReader::~HttpReader() {}

HttpWriter::HttpWriter() : m_file_fd(-1), m_socket_fd(-1) {}

int HttpWriter::open_file(std::string_view file_path) {
    (void)file_path;
    (void)m_file_fd;
    (void)m_socket_fd;
    (void)buf;
    return 0;
}

void HttpWriter::reset(int socket) {
    m_socket_fd = socket;
    if (m_file_fd != -1) close(m_file_fd);
    m_file_fd = -1;
}

HttpWriter::~HttpWriter() {
    if (m_file_fd != -1) close(m_file_fd);
}