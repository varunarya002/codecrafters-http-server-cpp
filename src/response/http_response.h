#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include <string>

class HttpResponse {
public:
    HttpResponse(const std::string &message, const int status_code, const std::string &content_type, const int content_length, const std::string &body):
    message(message), status_code(status_code), body(body), content_type(content_type), content_length(content_length) {}
    [[nodiscard]] std::string sendResponse() const {
        std::string WHITESPACE_DELIMITER = " ";
        std::string CARRIAGE_DELIMITER = "\r\n";
        std::string COLON_DELIMITER = ":";
        std::string http_response = "HTTP/1.1";
        std::string CONTENT_TYPE = "Content-Type";
        std::string CONTENT_LENGTH = "Content-Length";

        http_response = http_response + WHITESPACE_DELIMITER + std::to_string(status_code);
        http_response = http_response + WHITESPACE_DELIMITER + message;
        http_response = http_response + CARRIAGE_DELIMITER;

        //Add headers
        http_response = http_response + CONTENT_TYPE + COLON_DELIMITER + WHITESPACE_DELIMITER + content_type + CARRIAGE_DELIMITER;
        http_response = http_response + CONTENT_LENGTH + COLON_DELIMITER + WHITESPACE_DELIMITER + std::to_string(content_length) + CARRIAGE_DELIMITER;
        http_response = http_response + CARRIAGE_DELIMITER;

        //Add body
        http_response += body;

        return http_response;
    }
private:
    std::string message;
    int status_code;
    std::string body;
    std::string content_type;
    int content_length;
};


#endif //HTTP_RESPONSE_H
