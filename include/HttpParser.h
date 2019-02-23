/*
 * HttpParser.h
 * 
 *  Created on: Feb 20, 2019
 *      Author: xiagai
 */

#pragma once

#include "noncopyable.h"

#include <sys/stat.h>

namespace miniws {

class HttpParser : noncopyable {
public:
    static const int FILENAME_LEN = 200;
    //static const int WRITE_BUFFER_SIZE = 1024;

    enum METHOD {
        GET,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATCH
    };

    enum CHECK_STATE {
        CHECK_STATE_REQUESTLINE, //正在分析请求行
        CHECK_STATE_HEADER, //正在分析头部字段
        CHECK_STATE_CONTENT //正在分析请求体
    };

    enum HTTP_CODE {
        NO_REQUEST, //请求不完整，需要继续读取客户数据
        GET_REQUEST, //获得了一个完整的客户请求
        BAD_REQUEST, //客户请求有语法错误
        NO_RESOURCE, //资源不存在
        FORBIDDEN_REQUEST, //客户对资源没有足够的访问权限
        FILE_REQUEST, //文件请求
        INTERNAL_ERROR, //服务器内部错误
        CLOSED_CONNECTION //客户端已经关闭连接
    };

    enum LINE_STATUS {
        LINE_OK, //读到一个完整的行
        LINE_BAD, //行出错
        LINE_OPEN //行数据尚且不完整
    };

public:
    HttpParser(char *readBuf);
    ~HttpParser();

    HTTP_CODE processRead();

private:
    // used by read
    LINE_STATUS parseLine();
    char *getLine();
    HTTP_CODE parseRequestLine(char *text);
    HTTP_CODE parseHeaders(char *text);
    HTTP_CODE parseContent(char *text);
    HTTP_CODE doRequest();

    // used by write

private:
    CHECK_STATE m_checkState;
    METHOD m_method;

    char *m_readBuf;
    int m_readIdx;
    int m_checkedIdx;
    int m_startLine;

    char *m_url;
    char *m_version;
    char *m_host;
    int m_contentLen;
    bool m_linger;

    char m_readFile[FILENAME_LEN];
    struct stat m_fileStat;
};

}