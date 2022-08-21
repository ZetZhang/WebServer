/**
 * @file httpconn.h
 * @brief  http conn
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-21
 */
#ifndef __HTTPCONN_H__
#define __HTTPCONN_H__

#include <cstdlib>          // atoi()

#include <arpa/inet.h>    // sockaddr_in
#include <sys/types.h>
#include <sys/uio.h>        // readv / writev

#include "httprequest.h"
#include "httpresponse.h"

namespace wsv
{

class HttpConn
{
public:
    HttpConn();
    ~HttpConn();

    void init(int sockFd, const sockaddr_in &addr);

    ssize_t read(int *saveErrno);
    ssize_t write(int *saveErrno);

    void close();

    int getFd() const;
    int getPort() const;
    const char* getIP() const;
    sockaddr_in getAddr() const;
    bool isKeepAlive() const;
    int toWriteBytes();

    bool process();

    static bool isET;
    static const char *srcDir;
    static std::atomic<int> userCount;

private:
    bool                _isClosed;
    int                 _fd;
    int                 _iovCnt;
    struct sockaddr_in  _addr;
    struct iovec        _iov[2];
    Buffer              _readBuff;
    Buffer              _writeBuff;
    HttpRequest         _request;
    HttpResponse        _response;
};

}

#endif // __HTTPCONN_H__
