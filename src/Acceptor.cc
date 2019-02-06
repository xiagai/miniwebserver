/*
 * Acceptor.cc
 *
 *  Created on: Feb 1, 2019
 *      Author: xiagai
 */

#include "Acceptor.h"

#include <unistd.h>

namespace miniws {

Acceptor::Acceptor(EventLoop *eventLoop, const InetAddr &listenAddr)
    : m_eventLoop(eventLoop),
      m_listenSocket(),
      m_listenChannel(eventLoop, m_listenSocket.getSocketfd()) {
    m_listenSocket.bindAddr(listenAddr);
    m_listenChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::setNewConnCallback(const NewConnCallback &cb) {
    m_newConnCb = cb;
}

void Acceptor::listen() {
    m_eventLoop->assertInLoopThread();
    m_listenSocket.listenConn();
    m_listenChannel.enableReading();
}

void Acceptor::handleRead() {
    m_eventLoop->assertInLoopThread();
    InetAddr peerAddr;
    //FIX ME file descriptor mo more
    int connfd = m_listenSocket.acceptConn(peerAddr);
    if (connfd >= 0) {
        if (m_newConnCb) {
            m_newConnCb(connfd, peerAddr);
        }
        else {
            close(connfd);
        }
    }
}

}