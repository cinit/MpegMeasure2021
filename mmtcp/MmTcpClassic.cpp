//
// Created by kinit on 2021-11-05.
//

#include <string>
#include <vector>
#include <cstdint>
#include <unistd.h>

#include "MmTcpClassic.h"

cv::Mat MmTcpClassic::readImage() {
    if (mSocketFd == -1) {
        return {};
    } else {
        char strLength[17] = {};
        if (readExactly(strLength, 16) != 16) {
            return {};
        }
        char *endpos = nullptr;
        long len = strtol(strLength, &endpos, 10);
        if (len > 0) {
            std::vector<uint8_t> data;
            data.resize(len);
            if (readExactly(&data[0], int(len)) != len) {
                return {};
            }
            return cv::imdecode(data, cv::IMREAD_COLOR);
        } else {
            return {};
        }
    }
}

void MmTcpClassic::close() {
    ::close(mSocketFd);
    mSocketFd = -1;
}

int MmTcpClassic::getSocket() const {
    return mSocketFd;
}

void MmTcpClassic::setSocket(int fd) {
    mSocketFd = fd;
}

int MmTcpClassic::readExactly(void *buffer, int length) {
    int r = 0;
    int i = 0;
    while ((i = int(read(mSocketFd, static_cast<char *>(buffer) + r, length - r))) > 0) {
        r += i;
    }
    if (r != length) {
        return -errno;
    } else {
        return r;
    }
}
