/*
 * Common.h
 * 
 *  Created on: Feb 3, 2019
 *      Author: xiagai
 */
#pragma once

#include <memory>
#include <functional>

namespace miniws {

class TcpConnection;
class Buffer;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void (const TcpConnectionPtr &)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr &, Buffer &buf)> MessageCallback;
typedef std::function<void (const TcpConnectionPtr &)> CloseCallback;

static const int MAX_EVENT_NUM = 65536;

}