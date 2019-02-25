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
      m_channel(loop, sockfd, true),
      m_localAddr(localAddr),
      m_peerAddr(peerAddr),
      m_readBuf(READ_BUFFER_SIZE) {
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

void TcpConnection::setTcpNoDelay(bool on) {
    m_socket.setTcpNoDelay(on);
}

void TcpConnection::setTcpKeepAlive(bool on) {
    m_socket.setTcpKeepAlive(on);
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
    m_channel.remove();
}

void TcpConnection::sendv(struct iovec* iov, size_t iovlen) {
    m_loop->assertInLoopThread();
    struct msghdr msg = {};
    msg.msg_iov = iov;
    msg.msg_iovlen = iovlen;
    ssize_t n = ::sendmsg(m_socket.getSocketfd(), &msg, MSG_NOSIGNAL);
    if (n < 0) {
        char strerr[64];
		printf("LOG_ERR TcpConnection::sendv %s\n", strerror_r(errno, strerr, sizeof strerr));
    }
}

void TcpConnection::setState(StateE s) {
    m_state = s;
}

void TcpConnection::handleRead() {
    m_loop->assertInLoopThread();
    if (m_channel.isReadETMode()) {
        while (true) {
            //FIXME 有没有其他优雅的方式实现
            char *buf = new char[m_readBuf.remainingSize()];
            ssize_t n = ::recv(m_channel.fd(), buf, m_readBuf.remainingSize(), 0);
            if (n > 0) {
                m_readBuf.putIn(buf, static_cast<size_t>(n));
                if (m_readBuf.isFull()) {
                    //handle read buf
                    m_messageCb(shared_from_this(), m_readBuf);
                }
            }
            else if (n == 0 || (n < 0 && errno == EAGAIN)) {
                //handle read buf
                m_messageCb(shared_from_this(), m_readBuf);
                delete[] buf;
                handleClose();
                break;
            }
            else {
                delete[] buf;
                handleError();
                break;
            }
            delete[] buf;
        }
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
    m_loop->assertInLoopThread();
    int err = m_socket.getSocketError();
    printf("LOG_ERROR TcpConnection::handleError() %d\n", err);
}

}