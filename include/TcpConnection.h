/*
 * TcpConnection.h
 * 
 *  Created on: Feb 6, 2019
 *      Author: xiagai
 */

#pragma once 

#include "noncopyable.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddr.h"
#include "Common.h"

#include <memory>
#include <functional>
#include <string>

namespace miniws {

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop *loop,
                  const std::string &name,
                  int sockfd,
                  const InetAddr &localAddr,
                  const InetAddr &peerAddr);
    ~TcpConnection();
    std::string getName();
    void setConnectionCallback(const ConnectionCallback &cb);
    void setMessageCallback(const MessageCallback &cb);
    void setCloseCallback(const CloseCallback &cb);
    void connectEstablished();
    void connectDestroyed();
    EventLoop *getLoop();

private:
    enum StateE { kConnecting, kConnected, kDisconnected, };
private:
    void setState(StateE s);
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    EventLoop *m_loop;
    std::string m_name;
    StateE m_state;
    Socket m_socket;
    Channel m_channel;
    InetAddr m_localAddr;
    InetAddr m_peerAddr;
    ConnectionCallback m_connectionCb;
    MessageCallback m_messageCb;
    CloseCallback m_closeCb;
};

}