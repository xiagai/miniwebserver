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


}