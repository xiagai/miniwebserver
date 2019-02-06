/*
 * InetAddr.cc
 * 
 *  Created on: Feb 2, 2019
 *      Author: xiagai
 */

#include "InetAddr.h"

#include <arpa/inet.h>
#include <endian.h>
#include <assert.h>
#include <string.h>

namespace miniws {

InetAddr::InetAddr() {
    memset(&m_addr, 0, sizeof(m_addr));
}

InetAddr::InetAddr(std::string ip, uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    int ret = inet_pton(m_addr.sin_family, ip.c_str(), &(m_addr.sin_addr));
    if (ret <= 0) {
        printf("LOG_SYSERR InetAddr::InetAddr\n");
    }
    m_addr.sin_port = htobe16(port);
}

InetAddr::InetAddr(sockaddr_in &addr)
    : m_addr(addr) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr = addr;
}

InetAddr::~InetAddr() {}

const struct sockaddr_in *InetAddr::getAddr() const{
    return &m_addr;
}

std::string InetAddr::getIP() const {
    char ip[INET_ADDRSTRLEN] = "";
    const char *dst = inet_ntop(m_addr.sin_family, &(m_addr.sin_addr), ip, INET_ADDRSTRLEN);
    assert(dst != nullptr);
    return ip;
}

std::string InetAddr::getIPPort() const {
    std::string ipport;
    ipport = getIP();
    ipport.append(":" + std::to_string(getPort()));
    return ipport;
}

uint16_t InetAddr::getPort() const {
    return be16toh(m_addr.sin_port);
}

void InetAddr::setAddr(sockaddr_in &addr) {
    m_addr = addr;
}

}