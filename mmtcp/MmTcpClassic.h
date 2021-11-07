//
// Created by kinit on 2021-11-05.
//

#ifndef MPEGMEASURE2021_MMTCPCLASSIC_H
#define MPEGMEASURE2021_MMTCPCLASSIC_H

#include <opencv2/opencv.hpp>

class MmTcpClassic {
private:
    int mSocketFd = -1;

public:
    [[nodiscard]]
    int getSocket() const;

    void setSocket(int fd);

    [[nodiscard]]
    cv::Mat readImage();

    void close();

private:
    [[nodiscard]]
    int readExactly(void *buffer, int length);
};

#endif //MPEGMEASURE2021_MMTCPCLASSIC_H
