/**
 * @file log.h
 * @brief  日志
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#ifndef __LOG_H__
#define __LOG_H__

#include <thread>

#include <cstdarg>      // vastart va_end

#include <sys/stat.h>   // mkdir

#include "../buffer/buffer.h"
#include "blockqueue.h"

namespace wsv
{

class Log
{
public:
    static Log* Instance();
    static void FlushLogThread();

    void init(int level, const char *path = "./log", const char *suffix = ".log", int maxQueeuCapacity = 1024);

    int getLevel();
    void setLevel(int level);
    bool isOpen();

    void write(int level, const char *format, ...);
    void flush();

private:
    Log();
    virtual ~Log();
    void _asyncWrite();

private:
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    bool                                     _isAsync;
    bool                                     _isOpen;
    int                                      _lineCount;
    int                                      _toDay;
    int                                      _level;
    std::mutex                               _mtx;
    FILE                                     *_fp;
    const char                               *_path;
    const char                               *_suffix;
    Buffer                                   _buff;
    std::unique_ptr<BlockDeque<std::string>> _deque;
    std::unique_ptr<std::thread>             _writeThread;
};

}

#define LOG_BASE(level, format, ...)                    \
    do {                                                \
        wsv::Log *log = Log::Instance();                \
        if (log->isOpen() && log->getLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__);   \
            log->flush();                               \
        }                                               \
    } while (0)
#define LOG_DEBUG(format, ...) do { LOG_BASE(0, format, ##__VA_ARGS__); } while (0)
#define LOG_INFO(format, ...) do { LOG_BASE(1, format, ##__VA_ARGS__); } while (0)
#define LOG_WARN(format, ...) do { LOG_BASE(2, format, ##__VA_ARGS__); } while (0)
#define LOG_ERROR(format, ...) do { LOG_BASE(3, format, ##__VA_ARGS__); } while (0)

#endif // __LOG_H__
