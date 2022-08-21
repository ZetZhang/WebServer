/**
 * @file httprequest.h
 * @brief  http request
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-20
 */
#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include <unordered_map>
#include <unordered_set>
#include <regex>

#include <errno.h>

#include "../log/log.h"
#include "../pool/sqlconnRAII.h"

namespace wsv
{

class HttpRequest
{
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH
    };
    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    HttpRequest();
    ~HttpRequest() = default;

    void init();
    bool parse(Buffer &buff);

    std::string path() const;
    std::string& path();
    std::string method() const;
    std::string version() const;
    std::string getPost(const std::string &key) const;
    std::string getPost(const char *key) const;

    bool isKeepAlive() const;

private:
    bool _parseRequestLine(const std::string &line);
    void _parseHeader(const std::string &line);
    void _parseBody(const std::string &line);

    void _parsePath();
    void _parsePost();
    void _parseFromUrlEncoded();

    static bool UserVerify(const std::string &name, const std::string &pwd, bool isLogin);
    static int converHex(char ch);

private:
    PARSE_STATE _state;
    std::string _method;
    std::string _path;
    std::string _version;
    std::string _body;
    std::unordered_map<std::string, std::string>        _header;
    std::unordered_map<std::string, std::string>        _post;
    static const std::unordered_set<std::string>        DEFAULT_HTML;
    static const std::unordered_map<std::string, int>   DEFAULT_HTML_TAG;
};

}

#endif // __HTTPREQUEST_H__
