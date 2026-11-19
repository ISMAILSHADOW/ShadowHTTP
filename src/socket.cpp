#include "socket.h"
#include "connection.h"
#include "constants.h"
#include <cstring>
#include <iostream>
#include <unistd.h>

// The class for the server's tcp socket.
// It supports bind only not connect as for now it acts as a server only.
TcpSocket::TcpSocket() : m_sock_fd(-1) {}

uint16_t TcpSocket::get_port() {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);

    if (getsockname(m_sock_fd, (struct sockaddr *)&addr, &len) == -1) {
        std::cerr << "getsockname failed: " << strerror(errno) << std::endl;
        return 0;
    }

    if (addr.ss_family == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in *)&addr;
        return ntohs(sin->sin_port);
    } else if (addr.ss_family == AF_INET6) {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&addr;
        return ntohs(sin6->sin6_port);
    }

    return 0;
}

bool TcpSocket::init(const char *ip_or_domain, const char *port) {
    struct addrinfo req{}, *p, *sock;
    req.ai_family = AF_UNSPEC;
    req.ai_socktype = SOCK_STREAM;

    int ret;
    ret = getaddrinfo(ip_or_domain, port, &req, &sock);
    if (ret) {
        std::cerr << "getaddrinfo() failed: ";
        if (ret == EAI_SYSTEM) {
            std::cerr << strerror(errno) << std::endl;
            return false;
        }
        std::cerr << gai_strerror(ret) << std::endl;
        return false;
    }

    for (p = sock; p != NULL; p = p->ai_next) {
        if ((m_sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) != -1) {
            int opt = 1;
            setsockopt(m_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            ret = bind(m_sock_fd, p->ai_addr, p->ai_addrlen);
            if (ret) {
                std::cerr << "Unable to bind to port " << port << std::endl;
                std::cerr << strerror(errno) << std::endl;
                close(m_sock_fd);
                m_sock_fd = -1;
                continue;
            }
            m_port = get_port();
            break;
        }
    }

    if (m_sock_fd == -1) {
        freeaddrinfo(sock);
        return false;
    }

    freeaddrinfo(sock);
    return true;
}

bool TcpSocket::start() {
    if (listen(m_sock_fd, CONNECTIONS_BACKLOG) != 0) {
        std::cerr << "Listen() failed on port: " << m_port << " " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

HttpConnection TcpSocket::accept_connection() {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    int sock_fd = accept(m_sock_fd, (struct sockaddr *)&addr, &len);
    if (sock_fd == -1)
        std::cerr << "Unable to accept connection on port: " << m_port << " " << strerror(errno) << std::endl;
    return sock_fd;
}

TcpSocket::~TcpSocket() {
    if (m_sock_fd != -1) {
        close(m_sock_fd);
    }
}
