/*
 * HttpServer.cc
 * 
 *  Created on: Feb 25, 2019
 *      Author: xiagai
 */

#include "HttpServer.h"
#include "HttpParser.h"
#include "TcpConnection.h"

#include <string.h>

namespace miniws {

HttpServer::HttpServer(std::string &serverName, InetAddr &localAddr, std::string &homeDir, int numThreads)
    : m_name(serverName),
      m_localAddr(localAddr),
      m_numThreads(numThreads) {
    if (homeDir.size() > MAX_HOME_SIZE) {
        printf("The home directory length should be less than 100 characters. And ./ directory will be used.\n");
        strncpy(m_homeDir, "./", MAX_HOME_SIZE);
    }
    else {
        strncpy(m_homeDir, homeDir.c_str(), MAX_HOME_SIZE);
    }
}

HttpServer::~HttpServer() {}

void HttpServer::start() {
    EventLoop baseLoop;
    TcpServer tcpServer(&baseLoop, m_name, 4, m_localAddr);
    tcpServer.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    tcpServer.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpServer.start();
    baseLoop.loop();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn) {
    printf("connCb\n");
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer &buf) {
    printf("messageCb\n");
    HttpParser httpParser(m_homeDir, buf);
    httpret data = httpParser.process();
    if (data.iovlen > 0) {
        conn->sendv(data.iov, data.iovlen);
    }
}

}