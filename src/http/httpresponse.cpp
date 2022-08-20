/**
 * @file httpresponse.cpp
 * @brief  
 *
 * @author Ichheit, <ichheit@outlook.com>
 * @version 
 * @date 2022-08-19
 */
#include "httpresponse.h"

namespace wsv
{

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const std::unordered_map<int, std::string> HttpResponse::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

HttpResponse::HttpResponse() : _isKeepAlive(false), _code(-1), _mmFile(nullptr), _path(""), _srcDir("") {  }
HttpResponse::~HttpResponse() { unMapFile(); }

char* HttpResponse::file() { return _mmFile; }
size_t HttpResponse::fileLen() const { return _mmFileStat.st_size; }
int HttpResponse::code() const { return _code; }

void HttpResponse::unMapFile() {
    if (_mmFile) {
        munmap(file(), fileLen());
        _mmFile = nullptr;
    }
}

void HttpResponse::init(const std::string &srcDir, std::string &path, bool iskeepAlive, int code) {
    if (srcDir == "") {
        LOG_ERROR("HttpResponse > init: srcDir is \"\"");
        exit(EXIT_FAILURE);
    }
    if (_mmFile) unMapFile();
    _code = code;
    _isKeepAlive = iskeepAlive;
    _path = path;
    _srcDir = srcDir;
    _mmFile = nullptr;
}

// TODO
void HttpResponse::makeResponse(Buffer &buff) {
    if (stat((_srcDir + _path).data(), &_mmFileStat) < 0 || S_ISDIR(_mmFileStat.st_mode))
        _code = 404;
    else if (!(_mmFileStat.st_mode & S_IROTH))
        _code = 403;
    else if (_code == -1)
        _code = 200;
    _errorHtml();
    _addStateLine(buff);
    _addHeader(buff);
    _addContent(buff);
}

void HttpResponse::errorContent(Buffer &buff, std::string message) {
    std::string body, status;
    body += "<html><title>Error</title><body bgcolor=\"ffffff\">";
    status = CODE_STATUS.count(_code) == 1 ? CODE_STATUS.find(_code)->second : "Bad Request";
    body += std::to_string(_code) + " : " + status + "\n"
        + "<p>" + message + "</p><hr><em>WebServer</em></body></html>";
    buff.append("Content-length: "  + std::to_string(body.size()) + "\r\n\r\n");
    buff.append(body);
}

void HttpResponse::_errorHtml() {
    if (CODE_PATH.count(_code) == 1) {
        _path = CODE_PATH.find(_code)->second;
        stat((_srcDir + _path).data(), &_mmFileStat);
    }
}

std::string HttpResponse::_getFileType() {
    std::string::size_type idx = _path.find_last_of('.');
    if (idx == std::string::npos)
        return "text/plain";
    std::string suffix = _path.substr(idx);
    if (SUFFIX_TYPE.count(suffix) == 1)
        return SUFFIX_TYPE.find(suffix)->second;
    return "text/plain";
}

void HttpResponse::_addStateLine(Buffer &buff) {
    std::string status;
    if (CODE_STATUS.count(_code) != 1)
        _code = 400;
    status = CODE_STATUS.find(_code)->second;
    buff.append("HTTP/1.1 " + std::to_string(_code) + " " + status + "\r\n");
}

void HttpResponse::_addHeader(Buffer &buff) {
    buff.append("Connection: ");
    if (_isKeepAlive) {
        buff.append("keep-alive\r\n");
        buff.append("keep-alive: max=6, timeout=120\r\n");
    } else {
        buff.append("close\r\n");
    }
    buff.append("Content-type: " + _getFileType() + "\r\n");
}

void HttpResponse::_addContent(Buffer &buff) {
    int srcFd = open((_srcDir + _path).data(), O_RDONLY);
    if (srcFd < 0) {
        errorContent(buff, "File NotFound!");
        return;
    }
}

}
