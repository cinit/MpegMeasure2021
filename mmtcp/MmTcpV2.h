//
// Created by kinit on 2021-11-07.
//

#ifndef MPEGMEASURE2021_MMTCPV2_H
#define MPEGMEASURE2021_MMTCPV2_H

#include <tuple>
#include <cstdint>
#include <opencv2/core/mat.hpp>

class MmTcpV2 {
public:
    using ImagePacketHeader = struct {
        uint32_t imageLength;
        uint32_t raedFrameCostMs;
        uint64_t readFrameStartTimestampMs;
    };
    static_assert(sizeof(ImagePacketHeader) == 16, "ImagePacketHeader size error");

private:
    int mSocketFd = -1;

public:
    [[nodiscard]]
    int getSocket() const;

    void setSocket(int fd);

    [[nodiscard]]
    int readImage(cv::Mat &image, uint64_t &frameTimestampMs, uint32_t &frameCostMs);

    [[nodiscard]]
    int writeImage(const cv::Mat &image, uint64_t frameTimestampMs, uint32_t frameCostMs);

    void close();

private:
    [[nodiscard]]
    int readExactly(void *buffer, int length);

};

#endif //MPEGMEASURE2021_MMTCPV2_H
