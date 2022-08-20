/**
 * @file httprequest.cpp
 * @brief  http request
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-20
 */
#include "httprequest.h"

namespace wsv
{

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML {
        "/index",
        "/register",
        "/login",
        "/welcome",
        "/video",
        "/picture",
};

const std::unordered_map<std::string, int> HttpRequest::DEFAULT_HTML_TAG {
        {"/register.html", 0}, 
        {"/login.html", 1},
};

HttpRequest::HttpRequest() : _state(REQUEST_LINE), _method(""), _path(""), _version(""), _body("") { _header.clear(); _post.clear(); }

void HttpRequest::init() {
    _state = REQUEST_LINE;
    _method = _path = _version = _body = "";
    _header.clear();
    _post.clear();
}

bool HttpRequest::parse(Buffer &buff) {
    const char CRLF[] = "\r\n";
    if (buff.readableBytes() <= 0)
        return false;
    while (buff.readableBytes() && _state != FINISH) {
        const char *lineEnd = std::search(buff.peek(), buff.beginWriteConst(), CRLF, CRLF+2);
        std::string line(buff.peek(), lineEnd);
        switch (_state) {
            case REQUEST_LINE:
                if (!_parseRequestLine(line))
                    return false;;
                _parsePath();
                break;
            case HEADERS:
                _parseHeader(line);
                if (buff.readableBytes() <= 2)
                    _state = FINISH;
                break;
            case BODY:
                _parseBody(line);
                break;
            default:
                break;
        }
        if (lineEnd == buff.beginWrite())
            break;
        buff.retrieveUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", _method.c_str(), _path.c_str(), _version.c_str());
    return true;
}

std::string HttpRequest::path() const { return _path; }

std::string& HttpRequest::path() { return _path; }

std::string HttpRequest::method() const { return _method; }

std::string HttpRequest::version() const { return _version; }

std::string HttpRequest::getPost(const std::string &key) const {
    if (key == "") {
        LOG_ERROR("HttpRequest > getPost: key is \"\"");
        return "";
    }
    if (_post.count(key) == 1)
        return _post.find(key)->second;
    return "";
}

std::string HttpRequest::getPost(const char *key) const {
    if (!key) {
        LOG_ERROR("HttpRequest > getPost: key is nullptr");
        return "";
    }
    if (_post.count(key) == 1)
        return _post.find(key)->second;
    return "";
}

bool HttpRequest::isKeepAlive() const {
    if (_header.count("Connection") == 1)
        return _header.find("Connection")->second == "keep-alive" && _version == "1.1";
    return false;
}

bool HttpRequest::_parseRequestLine(const std::string &line) {
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if (std::regex_match(line, subMatch, pattern)) {
        _method = subMatch[1];
        _path = subMatch[2];
        _version = subMatch[3];
        _state = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpRequest::_parseHeader(const std::string &line) {
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if (std::regex_match(line, subMatch, pattern))
        _header[subMatch[1]] = subMatch[2];
    else
        _state = BODY;
}

void HttpRequest::_parseBody(const std::string &line) {
    _body = line;
    _parsePost();
    _state = FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

void HttpRequest::_parsePath() {
    if (_path == "/")
        _path = "/index.html";
    else
        for (auto &item : DEFAULT_HTML)
            if (item == _path) {
                _path += ".html";
                break;
            }
}

void HttpRequest::_parsePost() {
    if(_method == "POST" && _header["Content-Type"] == "application/x-www-form-urlencoded") {
        _parseFromUrlEncoded();
        if(DEFAULT_HTML_TAG.count(_path)) {
            int tag = DEFAULT_HTML_TAG.find(_path)->second;
            LOG_DEBUG("Tag:%d", tag);
            if(tag == 0 || tag == 1) {
                if(UserVerify(_post["username"], _post["password"], tag == 1))
                    _path = "/welcome.html";
                else
                    _path = "/error.html";
            }
        }
    }
}

// FIXME
void HttpRequest::_parseFromUrlEncoded() {
    if(_body.size() == 0) { return; }

    std::string key, value;
    int i = 0, j = 0;

    for(int num = 0, n = _body.size(); i < n; i++)
        switch (_body[i]) {
            case '=':
                key = _body.substr(j, i - j);
                j = i + 1;
                break;
            case '+':
                _body[i] = ' ';
                break;
            case '%':
                num = converHex(_body[i + 1]) * 16 + converHex(_body[i + 2]);
                _body[i + 2] = num % 10 + '0';
                _body[i + 1] = num / 10 + '0';
                i += 2;
                break;
            case '&':
                value = _body.substr(j, i - j);
                j = i + 1;
                _post[key] = value;
                LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
                break;
            default:
                break;
        }
    if(_post.count(key) == 0 && j < i) {
        value = _body.substr(j, i - j);
        _post[key] = value;
    }
}

bool HttpRequest::UserVerify(const std::string &name, const std::string &pwd, bool isLogin) {
    if (name == "" || pwd == "") 
        return false;
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
    MYSQL* sql;
    SqlConnRAII scr(&sql,  SqlConnPool::Instance());
    if (!sql) {
        LOG_ERROR("HttpRequest > UserVerify: SqlConnRAII error");
        return false;
    }

    char order[256] = {0};
    /* 查询用户及密码 */
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("%s", order);
    if(mysql_query(sql, order))
        return false;
    MYSQL_RES *res = mysql_store_result(sql);
    mysql_num_fields(res);
    MYSQL_FIELD *fields = mysql_fetch_fields(res);

    bool flag = !isLogin ? true : false;
    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        std::string password(row[1]);
        /* 注册行为 且 用户名未被使用*/
        flag = isLogin ? (pwd == password ? true : false) : false;
    }
    mysql_free_result(res);
    /* 注册行为 且 用户名未被使用*/
    if(!isLogin && flag == true) {
        LOG_DEBUG("regirster!");
        memset(order, 0, 256);
        snprintf(order, 256, "INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG( "%s", order);
        if(mysql_query(sql, order)) { 
            LOG_DEBUG( "Insert error!");
            flag = false; 
        }
    }
    SqlConnPool::Instance()->freeConn(sql);
    LOG_DEBUG( "UserVerify success!!");
    return flag;
}

int HttpRequest::converHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

}
