/*
 * HttpServer.h
 * 
 *  Created on: Feb 25, 2019
 *      Author: xiagai
 */

#pragma once

#include "noncopyable.h"
#include "InetAddr.h"
#include "TcpServer.h"
#include "Buffer.h"
#include "Common.h"

#include <string>

namespace miniws {

class HttpServer : noncopyable {
public:
    HttpServer(std::string &serverName, InetAddr &localAddr, std::string &homeDir, int numThreads, double delayCloseSec);
    ~HttpServer();
    void start();
    
private:
    void onConnection(const TcpConnectionPtr conn);
    void onMessage(const TcpConnectionPtr conn, Buffer &buf);

private:
    static const int MAX_HOME_SIZE = 100;
    std::string m_name;
    InetAddr m_localAddr;
    char m_homeDir[MAX_HOME_SIZE];
    int m_numThreads;
    double m_delayCloseSec;
};

}