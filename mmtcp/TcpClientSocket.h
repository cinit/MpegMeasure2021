//
// Created by kinit on 2021-11-05.
//

#ifndef MPEGMEASURE2021_TCPCLIENTSOCKET_H
#define MPEGMEASURE2021_TCPCLIENTSOCKET_H

class TcpClientSocket {
private:
    int mSocketFd = -1;

public:
    TcpClientSocket() = default;

    ~TcpClientSocket();

    TcpClientSocket(TcpClientSocket const &) = delete;

    TcpClientSocket &operator=(TcpClientSocket const &) = delete;

    [[nodiscard]]
    int connectToIpV4(const char *ip, int port);

    [[nodiscard]]
    bool isOpen() const noexcept;

    [[nodiscard]]
    int getSocket();

    int detach();

    void close();
};

#endif //MPEGMEASURE2021_TCPCLIENTSOCKET_H
