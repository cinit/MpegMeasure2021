//
// Created by kinit on 2021/3/19.
//

#ifndef ENGINEERTRAINUPPERCTL_SERIALINTERFACE_H
#define ENGINEERTRAINUPPERCTL_SERIALINTERFACE_H

class SerialInterface {
public:

    virtual ~SerialInterface() = 0;

    [[nodiscard]]
    virtual bool isOpened() const = 0;

    virtual void close() = 0;

    virtual int transmit(const void *buf, int length) = 0;

    //异步非阻塞
    virtual int receive(void *buf, int maxLength) = 0;
};

#endif //ENGINEERTRAINUPPERCTL_SERIALINTERFACE_H
