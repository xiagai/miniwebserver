/*
 * TcpServer.h
 * 
 *  Created on: Feb 6, 2019
 *      Author: xiagai
 */

#pragma once

#include "noncopyable.h"
#include "InetAddr.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "Common.h"
#include "EventLoopThreadPool.h"

#include <map>
#include <string>

namespace miniws {

class TcpServer : noncopyable {
public:
    TcpServer(EventLoop *loop, const std::string name, int numThreads, const InetAddr &localAddr);
    ~TcpServer();

    void start();

    /// Not Thread safe
    void setConnectionCallback(const ConnectionCallback &cb);
    /// Not Thread safe
    void setMessageCallback(const MessageCallback &cb);

private:
    /// Not thread safe, but in loop
    void newConnection(int sockfd, const InetAddr &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

private:
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

    EventLoop *m_loop; // the acceptor loop
    const std::string m_name;
    EventLoopThreadPool m_eventLoopThreadPool;
    InetAddr m_localAddr;
    Acceptor m_acceptor;
    ConnectionCallback m_connectionCb;
    MessageCallback m_messageCb;
    bool m_started;
    int m_nextConnId;
    ConnectionMap m_connections;
};

}