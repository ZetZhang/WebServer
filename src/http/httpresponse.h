/**
 * @file httpresponse.h
 * @brief  httpresponse
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__

#include <unordered_map>

#include <fcntl.h>

#include <sys/mman.h>

#include "../log/log.h"

namespace wsv
{

class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    void init(const std::string &srcDir, std::string &path, bool iskeepAlive = false, int code = -1);
    void makeResponse(Buffer &buff);
    void unMapFile();
    char* file();
    size_t fileLen() const;
    void errorContent(Buffer &buff, std::string message);
    int code() const;

private:
    std::string _getFileType();
    void _errorHtml();

    void _addStateLine(Buffer &buff);
    void _addHeader(Buffer &buff);
    void _addContent(Buffer &buff);

private:
    bool        _isKeepAlive;
    int         _code;
    char        *_mmFile;
    struct stat _mmFileStat;
    std::string _path;
    std::string _srcDir;
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};

}

#endif // __HTTPRESPONSE_H__
