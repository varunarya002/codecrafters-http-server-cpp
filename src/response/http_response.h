#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include <sstream>
#include <string>
#include <utility>

class HttpResponse {
public:
    HttpResponse(const std::string &message, const int status_code, const std::string &content_type, size_t content_length, const std::string &body, std::unordered_map<std::string, std::string> headers):
    message(message), status_code(status_code), body(body), content_type(content_type), content_length(content_length), headers(std::move(headers)) {}
    [[nodiscard]] std::string sendResponse() const {
        std::string WHITESPACE_DELIMITER = " ", CARRIAGE_DELIMITER = "\r\n", COLON_DELIMITER = ":";
        std::ostringstream http_response;
        std::string CONTENT_TYPE = "Content-Type", CONTENT_LENGTH = "Content-Length", ACCEPT_ENCODING = "Accept-Encoding",
        CONTENT_ENCODING = "Content-Encoding";

        http_response << "HTTP/1.1" << WHITESPACE_DELIMITER << status_code << WHITESPACE_DELIMITER << message << CARRIAGE_DELIMITER;

        if (headers.find(ACCEPT_ENCODING) != headers.end()) {
            std::string requested_encoding = headers.at(ACCEPT_ENCODING);
            // Add headers
            for (const auto& server_encoding: supported_encodings) {
                if (requested_encoding == server_encoding) {
                    http_response << CONTENT_ENCODING << COLON_DELIMITER << WHITESPACE_DELIMITER << requested_encoding << CARRIAGE_DELIMITER;
                    break;
                }
            }
        }

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
    std::unordered_map<std::string, std::string> headers;
    std::vector<std::string> supported_encodings = { "gzip" };
};


#endif //HTTP_RESPONSE_H
