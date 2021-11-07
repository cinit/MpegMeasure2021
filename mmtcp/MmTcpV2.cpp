//
// Created by kinit on 2021-11-07.
//

#include <cerrno>
#include <unistd.h>
#include <opencv2/imgcodecs.hpp>

#include "MmTcpV2.h"

int MmTcpV2::readImage(cv::Mat &image, uint64_t &frameTimestampMs, uint32_t &frameCostMs) {
    if (mSocketFd == -1) {
        return -EINVAL;
    } else {
        ImagePacketHeader header = {};
        if (int err; (err = readExactly(&header, 16)) != 16) {
            return err;
        }
        long len = header.imageLength;
        if (len > 0) {
            std::vector<uint8_t> data;
            data.resize(len);
            if (readExactly(&data[0], int(len)) != len) {
                return -EIO;
            }
            image = cv::imdecode(data, cv::IMREAD_COLOR);
            frameTimestampMs = header.readFrameStartTimestampMs;
            frameCostMs = header.raedFrameCostMs;
            return 0;
        } else {
            return -EIO;
        }
    }
}

int MmTcpV2::writeImage(const cv::Mat &image, uint64_t frameTimestampMs, uint32_t frameCostMs) {
    std::vector<uchar> buffer;
    if (cv::imencode(".jpg", image, buffer, {cv::IMWRITE_JPEG_QUALITY, 100})) {
        ImagePacketHeader header = {uint32_t(buffer.size()), frameCostMs, frameTimestampMs};
        if (::write(mSocketFd, &header, sizeof(ImagePacketHeader)) < 0) {
            return -errno;
        }
        if (::write(mSocketFd, &buffer[0], buffer.size()) < 0) {
            return -errno;
        }
        return 0;
    } else {
        return -EINVAL;
    }
}

void MmTcpV2::close() {
    ::close(mSocketFd);
    mSocketFd = -1;
}

int MmTcpV2::getSocket() const {
    return mSocketFd;
}

void MmTcpV2::setSocket(int fd) {
    mSocketFd = fd;
}

int MmTcpV2::readExactly(void *buffer, int length) {
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
