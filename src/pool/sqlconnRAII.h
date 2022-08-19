/**
 * @file sqlconnRAII.h
 * @brief  资源对象RAII
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#ifndef __SQLCONNRAII_H__
#define __SQLCONNRAII_H__

#include "sqlconnpool.h"

namespace wsv
{

class SqlConnRAII
{
public:
    SqlConnRAII(MYSQL **sql, SqlConnPool *connPool) : _sql(*sql), _connPool(connPool) {
        if (!connPool) {
            LOG_ERROR("connPool is nullptr");
            exit(EXIT_FAILURE);
        }
        *sql = connPool->getConn();
        _sql = *sql;
    }

    ~SqlConnRAII() { if (_sql) _connPool->freeConn(_sql); }

private:
    MYSQL *_sql;
    SqlConnPool *_connPool;
};

}

#endif // __SQLCONNRAII_H__
