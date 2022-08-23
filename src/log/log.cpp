/**
 * @file log.cpp
 * @brief  日志
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#include "log.h"

namespace wsv
{

Log::Log() : _isAsync(false), _isOpen(false), _lineCount(0), _toDay(0), _level(0), _fp(nullptr), _buff() { }

Log::~Log() {
    if (_writeThread && _writeThread->joinable()) {
        while (!_deque->empty())
            _deque->flush();
        _deque->close();
        _writeThread->join();
    }
    if (_fp) {
        std::lock_guard<std::mutex> locker(_mtx);
        Log::flush();
        fclose(_fp);
    }
}

Log* Log::Instance() {
    static Log instance;
    return &instance;
}

void Log::FlushLogThread() {
    Log::Instance()->_asyncWrite();
}

void Log::_asyncWrite() {
    std::string str;
    while (_deque->pop(str)) {
        std::lock_guard<std::mutex> locker(_mtx);
        fputs(str.c_str(), _fp);
    }
}

// FIXME: LOG_PATH_LEN
void Log::init(int level, const char *path, const char *suffix, int maxQueeuCapacity) {
    _isOpen = true;
    _level = level;
    _lineCount = 0;

    if (maxQueeuCapacity > 0) {
        _isAsync = true;
        if (!_deque) {
            std::unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
            _deque = std::move(newDeque);
            std::unique_ptr<std::thread> newThread(new std::thread(FlushLogThread));
            _writeThread = std::move(newThread);
        }
    } else {
        _isAsync = false;
    }

    char fileName[LOG_NAME_LEN] = {0};
    time_t timer = time(nullptr);
    struct tm *sysTime{localtime(&timer)};
    _path = path;
    _suffix = suffix;
    _toDay = sysTime->tm_mday;
    snprintf(fileName, LOG_NAME_LEN-1, "%s/%04d_%02d_%02d%s",
        _path, sysTime->tm_year+1900, sysTime->tm_mon+1, sysTime->tm_mday, _suffix);

    // lock
    {
        std::lock_guard<std::mutex> locker(_mtx);
        _buff.retrieveAll();
        if (_fp) {
            flush();
            fclose(_fp);
        }
        if ((_fp = fopen(fileName, "a")) == nullptr) {
            mkdir(_path, 0777);
            _fp = fopen(fileName, "a");
        }
        if (_fp == nullptr) {
            std::cout << "test" << std::endl;
            fprintf(stderr, "[Log > init]: %s\n", "_fp == nullptr");
            exit(EXIT_FAILURE);
        }
    }
}

int Log::getLevel() {
    std::lock_guard<std::mutex> locker(_mtx);
    return _level;
}

void Log::setLevel(int level) {
    std::lock_guard<std::mutex> locker(_mtx);
    _level = level;
}

bool Log::isOpen() {
    return _isOpen;
}

void Log::flush() {
    if (_isAsync)
        _deque->flush();
    fflush(_fp);
}

void Log::write(int level, const char *format, ...) {
    struct timeval now{0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime{localtime(&tSec)};
    va_list vaList;

    // filename
    if (_toDay != sysTime->tm_mday || (_lineCount && (_lineCount%MAX_LINES == 0))) {
        std::unique_lock<std::mutex> locker(_mtx);
        locker.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[11]= {0};
        snprintf(tail, 11, "%04d_%02d_%02d", sysTime->tm_year+1900, sysTime->tm_mon+1, sysTime->tm_mday);

        if (_toDay != sysTime->tm_mday) {
            snprintf(newFile, LOG_NAME_LEN-22, "%s/%s%s", _path, tail, _suffix);
            _toDay = sysTime->tm_mday;
            _lineCount = 0;
        } else {
            snprintf(newFile, LOG_NAME_LEN-22, "%s/%s-%d%s", _path, tail, (_lineCount/MAX_LINES), _suffix);
        }
        locker.lock();
        flush();
        fclose(_fp);
        std::cout << tail << std::endl;
        std::cout << newFile << std::endl;
        _fp = fopen(newFile, "a");
        if (_fp == nullptr) {
            fprintf(stderr, "[Log > write]: %s\n", "_fp == nullptr");
            exit(EXIT_FAILURE);
        }
    }

    // lock
    {
        std::unique_lock<std::mutex> locker(_mtx);
        _lineCount++;
        int n = snprintf(_buff.beginWrite(), 28, "%04d-%02d-%02d %02d:%02d:%02d.%06ld",
                sysTime->tm_year+1900, sysTime->tm_mon+1, sysTime->tm_mday, sysTime->tm_hour, sysTime->tm_min, sysTime->tm_sec, now.tv_usec);
        _buff.hasWritten(n);
        switch(level) {
            case 0:
                _buff.append("[debug]: ", 9);
                break;
            case 1:
                _buff.append("[info] : ", 9);
                break;
            case 2:
                _buff.append("[warn] : ", 9);
                break;
            case 3:
                _buff.append("[error]: ", 9);
                break;
            default:
                _buff.append("[info] : ", 9);
                break;
        }
        va_start(vaList, format);
        int m = vsnprintf(_buff.beginWrite(), _buff.writableBytes(), format, vaList);
        va_end(vaList);

        _buff.hasWritten(m);
        _buff.append("\n\0", 2);

        if (_isAsync && _deque && !_deque->full()) {
            _deque->push_back(_buff.retrieveAllToStr());
        } else {
            fputs(_buff.peek(), _fp);
            _buff.retrieveAll();
        }
    }
}


}
