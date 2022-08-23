/**
 * @file webserver.h
 * @brief  web server
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-21
 */
#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include <netinet/in.h>

#include "epoller.h"
#include "../log/log.h"
#include "../http/httpconn.h"
#include "../timer/heaptimer.h"
#include "../pool/sqlconnRAII.h"
#include "../pool/threadpool.h"

namespace wsv
{

class WebServer
{
public:
    WebServer(int port, int trigMode, int timeoutMS, bool optLinger,
            int sqlPort, const char *sqlUser, const char *sqlPwd,
            const char *dbName, int connPoolNum, int threadNum,
            bool openLog, int logLevel, int logQueueSize);
    ~WebServer();

    void start();

private:
    bool _initSocket();
    void _initEventMode(int trigMode);
    void _addClient(int fd, sockaddr_in addr);

    void _dealListen();
    void _dealWrite(HttpConn *client);
    void _dealRead(HttpConn *client);

    void _sendError(int fd, const char *info);
    void _extentTime(HttpConn *client);
    void _closeConn(HttpConn *client);

    void _onRead(HttpConn *client);
    void _onWrite(HttpConn *client);

    void _onProcess(HttpConn *client);

    static int setFdNonBlock(int fd);

private:
    bool _openLinger;
    bool _isClosed;
    int _port;
    int _listenFd;
    int _timeoutMS;
    uint32_t _listenEvent;
    uint32_t _connEvent;
    char *_srcDir;
    std::unique_ptr<HeapTimer> _timer;
    std::unique_ptr<ThreadPool> _threadPool;
    std::unique_ptr<Epoller> _epoller;
    std::unordered_map<int, HttpConn> _users;

    static const int MAX_FD = 65536;
};

}

#endif // __WEBSERVER_H__
