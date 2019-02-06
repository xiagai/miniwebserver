/*
 * Acceptor.h
 *
 *  Created on: Feb 1, 2019
 *      Author: xiagai
 */


#pragma once
#include "noncopyable.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddr.h"

#include <functional>

namespace miniws {

class Acceptor : noncopyable {
public:
    typedef std::function<void(int sockfd, const InetAddr &)> NewConnCallback;
    
    Acceptor(EventLoop *eventLoop, const InetAddr &listenAddr);
    void setNewConnCallback(const NewConnCallback &cb);
    void listen();

private:
    void handleRead();

    EventLoop *m_eventLoop;
    Socket m_listenSocket;
    Channel m_listenChannel;
    NewConnCallback m_newConnCb;
};

}