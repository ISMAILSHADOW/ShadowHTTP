#include "connection.h"
#include "socket.h"
#include <array>
#include <iostream>

std::array<HttpConnection, 10> connections;

int main() {
    TcpSocket server;
    if (!server.init("127.0.0.1", "8080")) {
        std::cerr << "Failed to initialize tcp socket! Terminating." << std::endl;
        exit(1);
    }
    if (!server.start()) exit(1);

    connections[0].reset(server.accept_connection());

    while (true) {
        if (connections[0].receive() == HttpConnErrorCode::CONNECTION_CLOSED) {
            std::cerr << "Connection closed." << std::endl;
            connections[0].reset(server.accept_connection());
        }
        if (connections[0] == -1) exit(1);
    }
    return 0;
}