#pragma once

#include "connection.h"
#include <netdb.h>

// The class for the server's tcp socket.
// It supports bind only not connect as for now it acts as a server only.
class TcpSocket {
    int m_sock_fd;
    uint16_t m_port;
    uint16_t get_port();

  public:
    TcpSocket();

    bool init(const char *ip_or_domain = "127.0.0.1", const char *port = "0");

    bool start();

    HttpConnection accept_connection();

    ~TcpSocket();
};