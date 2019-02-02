/*
 * InetAddr.h
 * 
 *  Created on: Feb 2, 2019
 *      Author: xiagai
 */

#pragma once

#include <netinet/in.h>
#include <string>

namespace miniws {

class InetAddr {
public:
    InetAddr(std::string ip, uint16_t port);
    explicit InetAddr(sockaddr_in &addr);
    ~InetAddr();

    std::string getIP () const;
    std::string getIPPort() const;
    u_int16_t getPort() const;
    const struct sockaddr_in *getAddr() const;

public:
    const static int kPORTSTRLEN = 10;

private:
    struct sockaddr_in m_addr;
};

}