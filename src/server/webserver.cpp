/**
 * @file webserver.cpp
 * @brief  web server
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-21
 */
#include  "webserver.h"

namespace wsv
{

WebServer::WebServer(int port, int trigMode, int timeoutMS, bool optLinger, int sqlPort, const char *sqlUser, const char *sqlPwd,
        const char *dbName, int connPoolNum, int threadNum, bool openLog, int logLevel, int logQueueSize) 
    : _openLinger(optLinger), _isClosed(false), _port(port), _timeoutMS(timeoutMS), _srcDir(getcwd(nullptr, 256)),
    _timer(new HeapTimer()), _epoller(new Epoller()), _threadPool(new ThreadPool(threadNum)) {
    if (!_srcDir)
        exit(EXIT_FAILURE);
    strncat(_srcDir, "/resources/", 16);
    HttpConn::userCount = 0;
    HttpConn::srcDir = _srcDir;
    SqlConnPool::Instance()->init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);

    _initEventMode(trigMode);
    if(!_initSocket()) _isClosed = true;

    if(openLog) {
        Log::Instance()->init(logLevel, "./log", ".log", logQueueSize);
        if(_isClosed) { LOG_ERROR("========== Server init error!=========="); }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", _port, optLinger? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (_listenEvent & EPOLLET ? "ET": "LT"),
                            (_connEvent & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}
WebServer::~WebServer() {
    close(_listenFd);
    _isClosed = true;
    free(_srcDir);
    SqlConnPool::Instance()->closePool();
}

void WebServer::start() {
    int timeMS = -1; // epoll wait timeout == -1 无事件将阻塞
    if(!_isClosed) { LOG_INFO("========== Server start =========="); }
    while(!_isClosed) {
        if (_timeoutMS > 0)
            timeMS = _timer->getNextTick();
        int eventCnt = _epoller->wait(timeMS);
        for(int i = 0; i < eventCnt; i++) {
            // 处理事件
            int fd = _epoller->getEventFd(i);
            uint32_t events = _epoller->getEvents(i);
            if(fd == _listenFd) {
                _dealListen();
            } else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(_users.count(fd) > 0);
                _closeConn(&_users[fd]);
            } else if(events & EPOLLIN) {
                assert(_users.count(fd) > 0);
                _dealRead(&_users[fd]);
            } else if(events & EPOLLOUT) {
                assert(_users.count(fd) > 0);
                _dealWrite(&_users[fd]);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

bool WebServer::_initSocket() {
    int ret;
    struct sockaddr_in addr;
    if(_port > 65535 || _port < 1024) {
        LOG_ERROR("Port:%d error!",  _port);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(_port);
    struct linger optLinger = { 0 };
    if(_openLinger) {
        // 优雅关闭: 直到所剩数据发送完毕或超时
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(_listenFd < 0) {
        LOG_ERROR("Create socket error!", _port);
        return false;
    }

    ret = setsockopt(_listenFd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(_listenFd);
        LOG_ERROR("Init linger error!", _port);
        return false;
    }

    int optval = 1;
    // 端口复用
    // 只有最后一个套接字会正常接收数据
    ret = setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(_listenFd);
        return false;
    }

    ret = bind(_listenFd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!", _port);
        close(_listenFd);
        return false;
    }

    ret = listen(_listenFd, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", _port);
        close(_listenFd);
        return false;
    }
    ret = _epoller->addFd(_listenFd,  _listenEvent | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(_listenFd);
        return false;
    }
    setFdNonBlock(_listenFd);
    LOG_INFO("Server port:%d", _port);
    return true;
}

void WebServer::_initEventMode(int trigMode) {
    _listenEvent = EPOLLRDHUP;
    _connEvent = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode) {
        case 0:
            break;
        case 1:
            _connEvent |= EPOLLET;
            break;
        case 2:
            _listenEvent |= EPOLLET;
            break;
        default:
            _listenEvent |= EPOLLET;
            _connEvent |= EPOLLET;
            break;
    }
    HttpConn::isET = (_connEvent & EPOLLET);
}

void WebServer::_addClient(int fd, sockaddr_in addr) {
    assert(fd > 0);
    _users[fd].init(fd, addr);
    if(_timeoutMS > 0) {
        _timer->add(fd, _timeoutMS, std::bind(&WebServer::_closeConn, this, &_users[fd]));
    }
    _epoller->addFd(fd, EPOLLIN | _connEvent);
    setFdNonBlock(fd);
    LOG_INFO("Client[%d] in!", _users[fd].getFd());
}

void WebServer::_dealListen() {
 struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(_listenFd, (struct sockaddr *)&addr, &len);
        if(fd <= 0) { return;}
        else if(HttpConn::userCount >= MAX_FD) {
            _sendError(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        _addClient(fd, addr);
    } while(_listenEvent & EPOLLET);
}

void WebServer::_dealWrite(HttpConn *client) {
    assert(client);
    _extentTime(client);
    _threadPool->addTask(std::bind(&WebServer::_onWrite, this, client));
}

void WebServer::_dealRead(HttpConn *client) {
 assert(client);
    _extentTime(client);
    _threadPool->addTask(std::bind(&WebServer::_onRead, this, client));
}

void WebServer::_sendError(int fd, const char *info) {
 assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::_extentTime(HttpConn *client) {
 assert(client);
    if(_timeoutMS > 0) { _timer->adjust(client->getFd(), _timeoutMS); }
}

void WebServer::_closeConn(HttpConn *client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->getFd());
    _epoller->delFd(client->getFd());
    client->close();
}

void WebServer::_onRead(HttpConn *client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        _closeConn(client);
        return;
    }
    _onProcess(client);
}

void WebServer::_onWrite(HttpConn *client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->toWriteBytes() == 0) {
        // 传输完成
        if(client->isKeepAlive()) {
            _onProcess(client);
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {
            // 继续传输
            _epoller->modFd(client->getFd(), _connEvent | EPOLLOUT);
            return;
        }
    }
    _closeConn(client);
}

void WebServer::_onProcess(HttpConn *client) {
    if(client->process()) {
        _epoller->modFd(client->getFd(), _connEvent | EPOLLOUT);
    } else {
        _epoller->modFd(client->getFd(), _connEvent | EPOLLIN);
    }
}

int WebServer::setFdNonBlock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

}
