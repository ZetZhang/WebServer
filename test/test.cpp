/**
 * @file test.cpp
 * @brief  test
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-24
 */
#include "../src/log/log.h"
#include "../src/pool/threadpool.h"
#include <features.h>

#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

void TestLog() {
    int cnt = 0, level = 0;
    wsv::Log::Instance()->init(level, "./TestLog_1", ".log", 0);
    for(level = 3; level >= 0; level--) {
        wsv::Log::Instance()->setLevel(level);
        for(int j = 0; j < 10000; j++ ){
            for(int i = 0; i < 4; i++) {
                LOG_BASE(i,"%s 111111111 %d ============= ", "Test", cnt++);
            }
        }
    }
    cnt = 0;
    wsv::Log::Instance()->init(level, "./TestLog_2", ".log", 5000);
    for(level = 0; level < 4; level++) {
        wsv::Log::Instance()->setLevel(level);
        for(int j = 0; j < 10000; j++ ){
            for(int i = 0; i < 4; i++) {
                LOG_BASE(i,"%s 222222222 %d ============= ", "Test", cnt++);
            }
        }
    }
}

void ThreadLogTask(int i, int cnt) {
    for(int j = 0; j < 10000; j++ ){
        LOG_BASE(i,"PID:[%04d]======= %05d ========= ", gettid(), cnt++);
    }
}

void TestThreadPool() {
    wsv::Log::Instance()->init(0, "./TestThreadPool", ".log", 5000);
    wsv::ThreadPool threadpool(6);
    for(int i = 0; i < 18; i++) {
        threadpool.addTask(std::bind(ThreadLogTask, i % 4, i * 10000));
    }
    getchar();
}

int main() {
    TestLog();
    TestThreadPool();
}
