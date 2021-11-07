//
// Created by kinit on 2021-11-05.
//
#include <chrono>
#include <cerrno>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>

#include "Time.h"

uint64_t getRelativeTimeMs() {
    return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

int getCpuTemperature() {
    int thermalFd;
    thermalFd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
    if (thermalFd < 0) {
        return -errno;
    }
    char buf[32] = {};
    if (read(thermalFd, buf, 32) < 0) {
        int err = errno;
        close(thermalFd);
        return -err;
    }
    close(thermalFd);
    char *outpos = nullptr;
    return int(strtol(buf, &outpos, 10));
}

bool testIpPort(const char *ip, uint16_t port) {
    int fd = 0;
    struct sockaddr_in addr;
    fd_set fdr, fdw;
    struct timeval timeout;
    int err = 0;
    int errlen = sizeof(err);
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return false;
    }
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    /*设置套接字为非阻塞*/
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        close(fd);
        return false;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        close(fd);
        return false;
    }
    /*阻塞情况下linux系统默认超时时间为75s*/
    int rc = connect(fd, (struct sockaddr *) &addr, sizeof(addr));
    if (rc != 0) {
        if (errno == EINPROGRESS) {
            /*正在处理连接*/
            FD_ZERO(&fdr);
            FD_ZERO(&fdw);
            FD_SET(fd, &fdr);
            FD_SET(fd, &fdw);
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;
            rc = select(fd + 1, &fdr, &fdw, nullptr, &timeout);
            /*select调用失败*/
            if (rc < 0) {
                close(fd);
                return false;
            }
            /*连接超时*/
            if (rc == 0) {
                close(fd);
                return false;
            }
            /*[1] 当连接成功建立时，描述符变成可写,rc=1*/
            if (rc == 1 && FD_ISSET(fd, &fdw)) {
                close(fd);
                return true;
            }
            /*[2] 当连接建立遇到错误时，描述符变为即可读，也可写，rc=2 遇到这种情况，可调用getsockopt函数*/
            if (rc == 2) {
                if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, reinterpret_cast<socklen_t *>(&errlen)) == -1) {
                    close(fd);
                    return false;
                }
                if (err) {
                    errno = err;
                    close(fd);
                    return false;
                }
            }
        }
    }
    return false;
}
