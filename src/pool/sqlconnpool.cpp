/**
 * @file sqlconnpool.cpp
 * @brief  数据库连接池
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#include "sqlconnpool.h"

namespace wsv
{

SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool connPool;
    return &connPool;
}

SqlConnPool::SqlConnPool() : _maxConn(0)/*, _useCount(0), _freeCount(0)*/ { }

SqlConnPool::~SqlConnPool() { closePool(); }

int SqlConnPool::Init(const char *host, int port, const char *user, const char *pwd, const char *dbName, int connSize) {
    if (connSize <= 0) {
        LOG_ERROR("connSize <= 0");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < connSize; i++) {
        MYSQL *sql = nullptr;
        if (!(sql = mysql_init(sql))) {
            LOG_ERROR("MySql init error!");
            exit(EXIT_FAILURE);
        }
        if (!(sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0))) {
            LOG_ERROR("MySql Connect error!");
            --connSize;
            break;
        }
        _connQueue.push(sql);
    }
    _maxConn = connSize;
    sem_init(&_semId, 0, _maxConn);
    return _maxConn;
}

MYSQL* SqlConnPool::getConn() {
    MYSQL *sql = nullptr;
    if (_connQueue.empty()) {
        LOG_WARN("SqlConnPool busy!");
        return nullptr;
    }
    sem_wait(&_semId);
    { // lock
        std::lock_guard<std::mutex> locker(_mtx);
        sql = _connQueue.front();
        _connQueue.pop();
    }
    return sql;
}

void SqlConnPool::freeConn(MYSQL *conn) {
    if (!conn) {
        LOG_ERROR("SqlConn is nullptr");
        exit(EXIT_FAILURE);
    }
    std::lock_guard<std::mutex> locker(_mtx);
    _connQueue.push(conn); // 丢弃
    sem_post(&_semId);
}

void SqlConnPool::closePool() {
    std::lock_guard<std::mutex> locker(_mtx);
    while (!_connQueue.empty()) {
        auto item = _connQueue.front();
        _connQueue.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

}
