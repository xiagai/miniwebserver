/*
 * Socket.h
 *
 *  Created on: Feb 1, 2019
 *      Author: xiagai
 */

#pragma once
#include "noncopyable.h"
#include "InetAddr.h"

namespace miniws {

class Socket : noncopyable {
public:
    Socket();
    Socket(int sockfd);
    ~Socket();

    int getSocketfd() const;
    int getSocketError();
    void setTcpNoDelay(bool on);
    void setTcpKeepAlive(bool on);
    void bindAddr(const InetAddr &addr);
    void listenConn();
    int acceptConn(InetAddr &peerAddr);

private:
    const int m_socketfd;
};

}