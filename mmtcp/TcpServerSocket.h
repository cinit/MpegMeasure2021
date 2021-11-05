//
// Created by kinit on 2021-11-05.
//

#ifndef MPEGMEASURE2021_TCPSERVERSOCKET_H
#define MPEGMEASURE2021_TCPSERVERSOCKET_H

class TcpServerSocket {
public:
    TcpServerSocket() = default;

    ~TcpServerSocket();

    TcpServerSocket(TcpServerSocket const &) = delete;

    TcpServerSocket &operator=(TcpServerSocket const &) = delete;

    int setListenPort(int port);

    int accept();

    void close();

private:
    int mServerSocketFd = -1;
    int mListenPort = -1;
};

#endif //MPEGMEASURE2021_TCPSERVERSOCKET_H
