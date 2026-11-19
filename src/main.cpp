#include "connection.h"
#include "parser.h"
#include "socket.h"
#include <iostream>

int main() {
    HttpParser p("CONNECT Maho:123 HTTP/1.1\r\n");
    HttpState state;
    if (auto ret = p.parse_request_line(state); ret != HttpParserErrorCode::OK) {
        std::cerr << "Something wrong with parsing: " << (int)ret << std::endl;
    }
    // TcpSocket server;
    // if (!server.init("127.0.0.1", "8080")) {
    //     std::cerr << "Failed to initialize tcp socket! Terminating." << std::endl;
    //     exit(1);
    // }
    // if (!server.start()) exit(1);

    // HttpConnection conn = server.accept_connection();
    // while (true) {
    //     if (conn == -1) exit(1);
    //     if (conn.receive() == HttpConnErrorCode::CONNECTION_CLOSED)
    //         conn = server.accept_connection();
    // }
    return 0;
}