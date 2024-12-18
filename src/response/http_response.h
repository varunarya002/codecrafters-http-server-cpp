#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <zlib.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

class HttpResponse {
public:
    HttpResponse(const std::string& message, int status_code, const std::string& content_type, size_t content_length, std::string body, std::unordered_map<std::string, std::string> headers)
        : message(message), status_code(status_code), body(std::move(body)), content_type(content_type), content_length(content_length), headers(std::move(headers)) {}

    [[nodiscard]] std::string sendResponse() {
        std::ostringstream http_response;

        // Status line
        http_response << "HTTP/1.1" << WHITESPACE_DELIMITER << status_code << WHITESPACE_DELIMITER << message << CARRIAGE_DELIMITER;

        // Content-Encoding header if applicable
        if (const auto encoding = getSupportedEncodings(headers); !encoding.empty()) {
            http_response << CONTENT_ENCODING << COLON_DELIMITER << WHITESPACE_DELIMITER << encoding << CARRIAGE_DELIMITER;
            body = compressString(body);
            content_length = body.size();
        }

        // Content-Type and Content-Length headers
        http_response << CONTENT_TYPE << COLON_DELIMITER << WHITESPACE_DELIMITER << content_type << CARRIAGE_DELIMITER;
        http_response << CONTENT_LENGTH << COLON_DELIMITER << WHITESPACE_DELIMITER << content_length << CARRIAGE_DELIMITER;

        // Blank line to separate headers from body
        http_response << CARRIAGE_DELIMITER;

        // Body
        http_response << body;

        return http_response.str();
    }

private:
    std::string message;
    int status_code;
    mutable std::string body;
    std::string content_type;
    size_t content_length;
    std::unordered_map<std::string, std::string> headers;

    static constexpr const char* WHITESPACE_DELIMITER = " ";
    static constexpr const char* CARRIAGE_DELIMITER = "\r\n";
    static constexpr const char* COLON_DELIMITER = ":";
    static constexpr const char* CONTENT_TYPE = "Content-Type";
    static constexpr const char* CONTENT_LENGTH = "Content-Length";
    static constexpr const char* CONTENT_ENCODING = "Content-Encoding";
    static constexpr const char* ACCEPT_ENCODING = "Accept-Encoding";

    const std::vector<std::string> supported_encodings = { "gzip" };

    // Helper function to split a string by a delimiter and trim whitespace
    static std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> result;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (end != std::string::npos) {
            result.push_back(trim(str.substr(start, end - start)));
            start = end + 1;
            end = str.find(delimiter, start);
        }

        result.push_back(trim(str.substr(start)));
        return result;
    }

    // Helper function to trim leading/trailing whitespace
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        size_t last = str.find_last_not_of(" \t\n\r");

        if (first == std::string::npos || last == std::string::npos) {
            return "";
        }

        return str.substr(first, last - first + 1);
    }

    // Function to get supported encoding from the headers
    [[nodiscard]] std::string getSupportedEncodings(const std::unordered_map<std::string, std::string>& headers) const {
        auto it = headers.find(ACCEPT_ENCODING);

        if (it != headers.end()) {
            auto requested_encodings = split(it->second, ',');

            for (const auto& encoding : requested_encodings) {
                if (std::find(supported_encodings.begin(), supported_encodings.end(), encoding) != supported_encodings.end()) {
                    return encoding; // Return first matching encoding
                }
            }
        }

        return "";
    }

    static std::string compressString(const std::string& str, int compressionlevel = Z_BEST_COMPRESSION) {
        z_stream zs; // z_stream is zlib's control structure
        memset(&zs, 0, sizeof(zs));

        if (deflateInit2(&zs, compressionlevel, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            throw std::runtime_error("deflateInit failed while compressing.");
        }

        zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
        zs.avail_in = str.size();

        int ret;
        char outbuffer[32768];
        std::string outstring;

        // Compress the input string in chunks
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = deflate(&zs, Z_FINISH);

            if (outstring.size() < zs.total_out) {
                outstring.append(outbuffer, zs.total_out - outstring.size());
            }
        } while (ret == Z_OK);

        deflateEnd(&zs);

        if (ret != Z_STREAM_END) {
            throw std::runtime_error("Exception during zlib compression.");
        }

        return outstring;
    }

};

#endif // HTTP_RESPONSE_H
