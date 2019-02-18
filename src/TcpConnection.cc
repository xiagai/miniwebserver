/*
 * TcpConnection.cc
 * 
 *  Created on: Feb 6, 2019
 *      Author: xiagai
 */

#include "TcpConnection.h"

#include <unistd.h>
#include <assert.h>
#include <string.h>

namespace miniws {

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddr &localAddr, const InetAddr &peerAddr)
    : m_loop(loop),
      m_name(name),
      m_state(kConnecting),
      m_socket(sockfd),
      m_channel(loop, sockfd),
      m_localAddr(localAddr),
      m_peerAddr(peerAddr) {
    m_channel.setReadCallback(std::bind(&TcpConnection::handleRead, this));
    m_channel.setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    m_channel.setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    m_channel.setErrorCallback(std::bind(&TcpConnection::handleError, this));
    printf("LOG_DEBUG TcpConnection::ctor at fd= %d\n", sockfd);
}

TcpConnection::~TcpConnection() {
    printf("LOG_DEBUG TcpConnection::dtor at fd= %d\n", m_socket.getSocketfd());
    assert(m_state == kDisconnected);
}

std::string TcpConnection::getName() {
    return m_name;
}

EventLoop *TcpConnection::getLoop() {
    return m_loop;
}

void TcpConnection::setConnectionCallback(const ConnectionCallback &cb) {
    m_connectionCb = cb;
}

void TcpConnection::setMessageCallback(const MessageCallback &cb) {
    m_messageCb = cb;
}

void TcpConnection::setCloseCallback(const CloseCallback &cb) {
    m_closeCb = cb;
}

void TcpConnection::connectEstablished() {
    m_loop->assertInLoopThread();
    assert(m_state == kConnecting);
    setState(kConnected);
    m_channel.enableReading();
    m_connectionCb(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    m_loop->assertInLoopThread();
    assert(m_state == kConnected);
    setState(kDisconnected);
    m_channel.disableAll();
    m_connectionCb(shared_from_this());
    //FIX ME smart pointer channel
    m_loop->removeChannel(&m_channel);
}

void TcpConnection::send(const char *buf, ssize_t len) {
    m_loop->assertInLoopThread();
    assert(len != 0);
    ssize_t n = ::send(m_channel.fd(), buf, len, 0);
    if (n < 0) {
        int error = errno;
        switch(error) {
            case EAGAIN:
            case EBADF:
            case EDESTADDRREQ:
            case EFAULT:
            case EFBIG:
            case ENOSPC:
            case EPERM:
            case EPIPE:
            case EINVAL:
            case EDQUOT:
            case EINTR:
            case EIO:
            default:
                char strerr[64];
                printf("LOG_DEBUG TcpConnection::send %s\n", strerror_r(errno, strerr, sizeof strerr));
        }
        //FIX ME hanle error
    }
    else if (n == len) {
        printf("LOG_TRACE TcpConnection::send %d bytes.\n");
    }
    else {
        assert(n < len);
        printf("LOG_WARN TcpConnection::send disk device was filled.\n");
    }
}

void TcpConnection::setState(StateE s) {
    m_state = s;
}

void TcpConnection::handleRead() {
    char buf[65536];
    ssize_t n = ::recv(m_channel.fd(), buf, sizeof buf, 0);
    if (n > 0) {
        //FIX ME n == 65536 buf cannot contain
        m_messageCb(shared_from_this(), buf, n);
    }
    else if (n == 0) {
        handleClose();
    }
    else {
        handleError();
    }
}

void TcpConnection::handleWrite() {}

void TcpConnection::handleClose() {
    m_loop->assertInLoopThread();
    printf("LOG_TRACE TcpConnection::handleClose state = %d\n", m_state);
    assert(m_state == kConnected);
    m_channel.disableAll();
    m_closeCb(shared_from_this());
}

void TcpConnection::handleError() {
    int err = m_socket.getSocketError();
    printf("LOG_ERROR TcpConnection::handleError() %d\n", err);
}

}