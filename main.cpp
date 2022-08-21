#include <iostream>

#include "src/buffer/buffer.h"
#include "src/log/blockqueue.hpp"
#include "src/log/log.h"
#include "src/pool/sqlconnpool.h"
#include "src/pool/threadpool.h"
#include "src/pool/sqlconnRAII.h"
#include "src/timer/heaptimer.h"
#include "src/server/epoller.h"
#include "src/http/httpresponse.h"
#include "src/http/httprequest.h"
#include "src/http/httpconn.h"

int main(int argc, char *argv[])
{
    wsv::Buffer buff(10);
    buff.append("nihao", 12);
    // buff.retrieve(-1);

    wsv::Log *log = wsv::Log::Instance();

    wsv::BlockDeque<int> ok1(10);
    wsv::BlockDeque<std::string> ok2(10);

    LOG_INFO("dd");
    return 0;
}
