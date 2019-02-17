/*
 * TcpServer.cc
 * 
 *  Created on: Feb 6, 2019
 *      Author: xiagai
 */

#include "TcpServer.h"
#include "TcpConnection.h"

#include <assert.h>

namespace miniws {

TcpServer::TcpServer(EventLoop *loop, const std::string name, int numThreads, const InetAddr &localAddr)
    : m_loop(loop),
      m_name(name),
      m_eventLoopThreadPool(loop, numThreads),
      m_localAddr(localAddr),
      m_acceptor(loop, localAddr),
      m_started(false),
      m_nextConnId(0) {
    m_acceptor.setNewConnCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    m_loop->assertInLoopThread();
    for (auto &item : m_connections) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::start() {
    if (!m_started) {
        m_eventLoopThreadPool.start();
        m_loop->runInLoop(std::bind(&Acceptor::listen, &m_acceptor));
    }
}

void TcpServer::setConnectionCallback(const ConnectionCallback &cb) {
    m_connectionCb = cb;
}

void TcpServer::setMessageCallback(const MessageCallback &cb) {
    m_messageCb = cb;
}

void TcpServer::newConnection(int sockfd, const InetAddr &peerAddr) {
    m_loop->assertInLoopThread();
    EventLoop *ioLoop = m_eventLoopThreadPool.getNextLoop();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", m_nextConnId);
    ++m_nextConnId;
    std::string connName = m_name + buf;

    printf("LOG_INFO TcpServer::newConnection [%s] - new connection [%s] from %s\n", 
        m_name.c_str(), connName.c_str(), peerAddr.getIPPort().c_str());
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop, connName, sockfd, m_localAddr, peerAddr);
    m_connections[connName] = conn;
    conn->setConnectionCallback(m_connectionCb);
    conn->setMessageCallback(m_messageCb);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, conn));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    m_loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    m_loop->assertInLoopThread();
    printf("LOG_INFO TcpServer::removeConnection\n");
    size_t n = m_connections.erase(conn->getName());
    assert(n == 1);
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

}