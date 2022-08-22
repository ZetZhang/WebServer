#include "src/buffer/buffer.h"
#include "src/log/blockqueue.hpp"
// #include "src/log/log.h"
// #include "src/pool/sqlconnpool.h"
// #include "src/pool/threadpool.h"
// #include "src/pool/sqlconnRAII.h"
// #include "src/timer/heaptimer.h"
// #include "src/http/httpresponse.h"
// #include "src/http/httprequest.h"
// #include "src/http/httpconn.h"
// #include "src/server/epoller.h"
#include "src/server/webserver.h"

int main(int argc, char *argv[])
{
    // wsv::Buffer buff(10);
    // buff.append("nihao", 12);
    // buff.retrieve(-1);

    // wsv::Log *log = wsv::Log::Instance();
    // log->init(1, "./log", ".log", 1000);
    // std::cout << log->isOpen() << std::endl;

    // wsv::Log::Instance()->init(1, "./log", ".log", 1000);
    // wsv::SqlConnPool::Instance()->init("localhost", 3306, "root", "zjt152445", "yourdb", 5);
    // wsv::HeapTimer *timer = new wsv::HeapTimer();
    // wsv::BlockDeque<int> ok1(10);
    // wsv::BlockDeque<std::string> ok2(10);

    // LOG_INFO("dd");

    // wsv::Epoller epoll(1);
    wsv::WebServer server( 1316, 3, 60000, false, 3306, "root", "zjt152445", "yourdb", 12, 6, true, 1, 1024);
    server.start();

    // wsv::HttpConn hc;
    return 0;
}
