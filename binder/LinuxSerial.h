//
// Created by kinit on 2021/3/19.
//

#ifndef ENGINEERTRAINUPPERCTL_LINUXSERIAL_H
#define ENGINEERTRAINUPPERCTL_LINUXSERIAL_H

#include "SerialInterface.h"

class LinuxSerial : public SerialInterface {
public:
    [[nodiscard]]
    int open(const char *name);

    [[nodiscard]]
    bool isOpened() const override;

    void close() override;

    int transmit(const void *buf, int length) override;

    int receive(void *buf, int maxLength) override;

private:
    int serialFd = -1;
};

#endif //ENGINEERTRAINUPPERCTL_LINUXSERIAL_H
