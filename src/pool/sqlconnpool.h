/**
 * @file sqlconnpool.h
 * @brief  数据库池
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#ifndef __SQLCONNPOOL_H__
#define __SQLCONNPOOL_H__

#include <queue>

#include <semaphore.h>

#include <mysql/mysql.h>

#include "../log/log.h"

namespace wsv
{

class SqlConnPool
{
public:
    static SqlConnPool* Instance();

    int init(const char *host, int port, const char *user, const char *pwd, const char *dbName, int connSize);

    MYSQL* getConn();
    void freeConn(MYSQL *conn);
    void closePool();

private:
    SqlConnPool();
    ~SqlConnPool();

private:
    int _maxConn;
    /* int _useCount; */
    /* int _freeCount; */
    sem_t _semId;
    std::mutex _mtx;
    std::queue<MYSQL*> _connQueue;
};

}

#endif // __SQLCONNPOOL_H__
