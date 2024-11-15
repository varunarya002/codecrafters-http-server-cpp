#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include <sstream>
#include <string>

class HttpResponse {
public:
    HttpResponse(const std::string &message, const int status_code, const std::string &content_type, size_t content_length, const std::string &body):
    message(message), status_code(status_code), body(body), content_type(content_type), content_length(content_length) {}
    [[nodiscard]] std::string sendResponse() const {
        std::string WHITESPACE_DELIMITER = " ";
        std::string CARRIAGE_DELIMITER = "\r\n";
        std::string COLON_DELIMITER = ":";
        std::ostringstream http_response;
        std::string CONTENT_TYPE = "Content-Type";
        std::string CONTENT_LENGTH = "Content-Length";

        http_response << "HTTP/1.1" << WHITESPACE_DELIMITER << status_code << WHITESPACE_DELIMITER << message << CARRIAGE_DELIMITER;

        // Add headers
        http_response << CONTENT_TYPE << COLON_DELIMITER << WHITESPACE_DELIMITER << content_type << CARRIAGE_DELIMITER;
        http_response << CONTENT_LENGTH << COLON_DELIMITER << WHITESPACE_DELIMITER << content_length << CARRIAGE_DELIMITER;

        // Add a blank line to separate headers from body
        http_response << CARRIAGE_DELIMITER;

        http_response << body;

        return http_response.str();
    }
private:
    std::string message;
    int status_code;
    std::string body;
    std::string content_type;
    size_t content_length;
};


#endif //HTTP_RESPONSE_H
