//
// Created by kinit on 2021-11-05.
//

#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "TcpClientSocket.h"

TcpClientSocket::~TcpClientSocket() {
    if (mSocketFd != -1) {
        ::close(mSocketFd);
        mSocketFd = -1;
    }
}

int TcpClientSocket::connectToIpV4(const char *ip, int port) {
    int socket_fd = 0;
    sockaddr_in server = {};
    //Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        return -errno;
    }
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(uint16_t(port));
    // Connect to remote server
    if (connect(socket_fd, (struct sockaddr *) &server, sizeof(server)) < 0) {
        int err = errno;
        ::close(socket_fd);
        return -err;
    }
    mSocketFd = socket_fd;
    return 0;
}

bool TcpClientSocket::isOpen() const noexcept {
    return mSocketFd > 0;
}

int TcpClientSocket::getSocket() {
    return mSocketFd;
}

int TcpClientSocket::detach() {
    int i = mSocketFd;
    mSocketFd = -1;
    return i;
}

void TcpClientSocket::close() {
    if (mSocketFd != -1) {
        ::close(mSocketFd);
        mSocketFd = -1;
    }
}
