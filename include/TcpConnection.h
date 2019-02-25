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
#include "Buffer.h"

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
    EventLoop *getLoop();

    void setConnectionCallback(const ConnectionCallback &cb);
    void setMessageCallback(const MessageCallback &cb);
    void setCloseCallback(const CloseCallback &cb);
    void setTcpNoDelay(bool on);
    void setTcpKeepAlive(bool on);
    void connectEstablished();
    void connectDestroyed();
    void sendv(struct iovec* iov, size_t iovlen);

private:
    enum StateE { kConnecting, kConnected, kDisconnected, };
    static const size_t READ_BUFFER_SIZE = 4096; //FIXME 修改为用户可自定义缓存大小
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
    Buffer m_readBuf;
    ConnectionCallback m_connectionCb;
    MessageCallback m_messageCb;
    CloseCallback m_closeCb;
};

}