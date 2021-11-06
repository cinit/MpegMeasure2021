//
// Created by kinit on 2021/3/19.
//

#include "LinuxSerial.h"

#include <fcntl.h>
#include <cerrno>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

constexpr auto BIT_RATE = B115200;

int LinuxSerial::open(const char *name) {
    serialFd = ::open(name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    //O_NONBLOCK设置为非阻塞模式，在read时不会阻塞住，在读的时候将read放在while循环中，下一节篇文档将详细讲解阻塞和非阻塞
    printf("fd=%d\n", serialFd);
    if (serialFd == -1) {
        int err = errno;
        printf("errno(%d):%s\n", err, strerror(err));
        perror("Can't Open SerialPort");
        return -err;
    }
    /*恢复串口为阻塞状态*/
//    if (fcntl(serialFd, F_SETFL, 0) < 0)
//        printf("fcntl failed!\n");
//    else
//        printf("fcntl=%d\n", fcntl(serialFd, F_SETFL, 0));
    struct termios newtio, oldtio;
    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
    if (tcgetattr(serialFd, &oldtio) != 0) {
        int err = errno;
        perror("SetupSerial");
        printf("tcgetattr( fd,&oldtio) -> %d\n", tcgetattr(serialFd, &oldtio));
        ::close(serialFd);
        serialFd = -1;
        return -err;
    }
    bzero(&newtio, sizeof(newtio));
    /*步骤一，设置字符大小*/
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;
    /*设置停止位*/
    newtio.c_cflag |= CS8;
    /*设置奇偶校验位*/
    /*奇数
    newtio.c_cflag |= PARENB;
    newtio.c_cflag |= PARODD;
    newtio.c_iflag |= (INPCK | ISTRIP);
    /*
    //偶数
    newtio.c_iflag |= (INPCK | ISTRIP);
    newtio.c_cflag |= PARENB;
    newtio.c_cflag &= ~PARODD;
    //无奇偶校验位
     */
    newtio.c_cflag &= ~PARENB;

    /*设置波特率*/
    cfsetispeed(&newtio, BIT_RATE);
    cfsetospeed(&newtio, BIT_RATE);
    /*设置停止位*/
    newtio.c_cflag &= ~CSTOPB;//1
//    newtio.c_cflag |= CSTOPB;//2
    /*设置等待时间和最小接收字符*/
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    /*处理未接收字符*/
    tcflush(serialFd, TCIFLUSH);
    /*激活新配置*/
    if ((tcsetattr(serialFd, TCSANOW, &newtio)) != 0) {
        int err = errno;
        perror("com set error");
        ::close(serialFd);
        serialFd = -1;
        return -err;
    }
    printf("set done!\n");
    return 0;
}

bool LinuxSerial::isOpened() const {
    return serialFd >= 0;
}

void LinuxSerial::close() {
    ::close(serialFd);
    serialFd = -1;
}

int LinuxSerial::transmit(const void *buf, int length) {
    return ::write(serialFd, buf, length);
}

//异步非阻塞
int LinuxSerial::receive(void *buf, int maxLength) {
    return ::read(serialFd, buf, maxLength);
}
