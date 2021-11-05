//
// Created by kinit on 2021-11-05.
//

#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "TcpServerSocket.h"

TcpServerSocket::~TcpServerSocket() {
    if (mServerSocketFd != -1) {
        ::close(mServerSocketFd);
        mServerSocketFd = -1;
    }
}

int TcpServerSocket::setListenPort(int port) {
    close();
    if (port < 0 || port > 65535) {
        return -EINVAL;
    }
    mServerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (mServerSocketFd < 0) {
        return -errno;
    }
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(uint16_t(port));
    if (bind(mServerSocketFd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        int err = errno;
        ::close(mServerSocketFd);
        return -err;
    }
    if (listen(mServerSocketFd, 2) < 0) {
        int err = errno;
        ::close(mServerSocketFd);
        return -err;
    }
    return 0;
}

int TcpServerSocket::accept() {
    sockaddr remote_addr = {};
    socklen_t addr_len = 0;
    int fd;
    if ((fd = ::accept(mServerSocketFd, &remote_addr, &addr_len)) < 0) {
        return -errno;
    }
    return fd;
}

void TcpServerSocket::close() {
    if (mServerSocketFd != -1) {
        ::close(mServerSocketFd);
        mServerSocketFd = -1;
    }
}
