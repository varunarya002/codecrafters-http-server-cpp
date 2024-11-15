#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include <string>
#include <unordered_map>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string request_param;
    std::unordered_map<std::string, std::string> headers;
    std::string directory_name;
    std::string body;
};

#endif //HTTP_REQUEST_H