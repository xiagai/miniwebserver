/*
 * Common.h
 * 
 *  Created on: Feb 3, 2019
 *      Author: xiagai
 */
#pragma once

#include <memory>

namespace miniws {

class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void (const TcpConnectionPtr &)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr &, const char *buf, ssize_t len)> MessageCallback;
typedef std::function<void (const TcpConnectionPtr &)> CloseCallback;

const static int MAX_EVENT_NUM = 65536;

}