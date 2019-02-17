/*
 * Socket.cc
 *
 *  Created on: Feb 1, 2019
 *      Author: xiagai
 */

#include "Socket.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

namespace miniws {

namespace detail {

int createSocketfd() {
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (fd < 0) {
        printf("LOG_SYSFATAL createSocketfd\n");
    }
    return fd;
}

}

Socket::Socket()
    : m_socketfd(detail::createSocketfd()) {
}

Socket::Socket(int sockfd)
    : m_socketfd(sockfd) {
}

Socket::~Socket() {
    close(m_socketfd);
}

int Socket::getSocketfd() const {
    return m_socketfd;
}

void Socket::bindAddr(const InetAddr &addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof(sockaddr));
    int ret = bind(m_socketfd, (sockaddr *)addr.getAddr(), addrlen);
    if (ret < 0) {
        printf("LOG_SYSFATAL bindAddr\n");
    }
}

void Socket::listenConn() {
    int ret = listen(m_socketfd, SOMAXCONN);
    if (ret < 0) {
        printf("LOG_SYSFATAL listenConn\n");
    }
}

int Socket::acceptConn(InetAddr &peerAddr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof(sockaddr));
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    int connfd = accept4(m_socketfd, (sockaddr *)&addr, &addrlen, SOCK_CLOEXEC | SOCK_NONBLOCK);
    if (connfd < 0) {
        printf("LOG_SYSFATAL acceptConn\n");
    }
    peerAddr.setAddr(addr);
    //FIXME errno switch
    return connfd;
}

int Socket::getSocketError() {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(sockaddr));

    if (getsockopt(m_socketfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    }
    else {
        return optval;
    }
}

}