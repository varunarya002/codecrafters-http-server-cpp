#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <zlib.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <algorithm>
#include <stdexcept>

/**
 * @class HttpResponse
 * @brief Represents an HTTP response with methods to build and serialize it
 * 
 * This class encapsulates all components of an HTTP response including status code,
 * headers, and body. It provides methods to build a valid HTTP response string
 * with support for content compression.
 */
class HttpResponse {
public:
    /**
     * @brief Constructs a new HTTP response with all necessary components
     * 
     * @param message Response status message (e.g., "OK", "Not Found")
     * @param status_code HTTP status code (e.g., 200, 404)
     * @param content_type MIME type of the response body
     * @param content_length Length of the response body in bytes
     * @param body Response body content
     * @param headers Additional HTTP headers
     */
    HttpResponse(
        std::string  message,
        int status_code,
        std::string  content_type,
        size_t content_length,
        std::string body,
        std::unordered_map<std::string, std::string> headers
    )
        : message_(std::move(message))
        , status_code_(status_code)
        , content_type_(std::move(content_type))
        , content_length_(content_length)
        , body_(std::move(body))
        , headers_(std::move(headers)) 
    {}

    /**
     * @brief Builds and returns the complete HTTP response as a string
     * 
     * @return std::string The formatted HTTP response
     */
    [[nodiscard]] std::string sendResponse() {
        std::ostringstream response_stream;
        
        appendStatusLine(response_stream);
        appendHeaders(response_stream);
        appendBody(response_stream);
        
        return response_stream.str();
    }

private:
    // HTTP response components
    std::string message_;
    int status_code_;
    std::string content_type_;
    size_t content_length_;
    mutable std::string body_; // Mutable to allow compression in const methods
    std::unordered_map<std::string, std::string> headers_;
    
    // HTTP format constants
    static constexpr const char* WHITESPACE_DELIMITER = " ";
    static constexpr const char* CARRIAGE_DELIMITER = "\r\n";
    static constexpr const char* COLON_DELIMITER = ":";
    
    // Common HTTP header names
    static constexpr const char* CONTENT_TYPE = "Content-Type";
    static constexpr const char* CONTENT_LENGTH = "Content-Length";
    static constexpr const char* CONTENT_ENCODING = "Content-Encoding";
    static constexpr const char* ACCEPT_ENCODING = "Accept-Encoding";
    static constexpr const char* CONNECTION = "Connection";
    
    // Supported compression encodings
    const std::vector<std::string> supported_encodings_ = { "gzip" };

    /**
     * @brief Appends the HTTP status line to the response stream
     * 
     * @param stream The output stream to append to
     */
    void appendStatusLine(std::ostringstream& stream) const {
        stream << "HTTP/1.1" << WHITESPACE_DELIMITER 
               << status_code_ << WHITESPACE_DELIMITER 
               << message_ << CARRIAGE_DELIMITER;
    }
    
    /**
     * @brief Appends all HTTP headers to the response stream
     * 
     * @param stream The output stream to append to
     */
    void appendHeaders(std::ostringstream& stream) {
        // Apply compression if supported by the client
        std::string encoding = getSupportedEncodings(headers_);
        if (!encoding.empty()) {
            stream << CONTENT_ENCODING << COLON_DELIMITER << WHITESPACE_DELIMITER 
                   << encoding << CARRIAGE_DELIMITER;
            body_ = compressString(body_);
            content_length_ = body_.size();
        }
        
        // Add standard headers
        stream << CONTENT_TYPE << COLON_DELIMITER << WHITESPACE_DELIMITER 
               << content_type_ << CARRIAGE_DELIMITER;
        stream << CONTENT_LENGTH << COLON_DELIMITER << WHITESPACE_DELIMITER 
               << content_length_ << CARRIAGE_DELIMITER;
        
        // Add Connection: close header if requested
        if (headers_.contains(CONNECTION) && headers_.at(CONNECTION) == "close") {
            stream << CONNECTION << COLON_DELIMITER << WHITESPACE_DELIMITER 
                   << "close" << CARRIAGE_DELIMITER;
        }
        
        // Add blank line to separate headers from body
        stream << CARRIAGE_DELIMITER;
    }
    
    /**
     * @brief Appends the response body to the stream
     * 
     * @param stream The output stream to append to
     */
    void appendBody(std::ostringstream& stream) const {
        stream << body_;
    }

    /**
     * @brief Splits a string by a delimiter and trims whitespace
     * 
     * @param str String to split
     * @param delimiter Character to split on
     * @return std::vector<std::string> Vector of trimmed substrings
     */
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

    /**
     * @brief Trims leading and trailing whitespace from a string
     * 
     * @param str String to trim
     * @return std::string Trimmed string
     */
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        size_t last = str.find_last_not_of(" \t\n\r");

        if (first == std::string::npos || last == std::string::npos) {
            return "";
        }

        return str.substr(first, last - first + 1);
    }

    /**
     * @brief Determines which encoding to use based on client preferences
     * 
     * @param headers Request headers containing encoding preferences
     * @return std::string Selected encoding or empty string if none supported
     */
    [[nodiscard]] std::string getSupportedEncodings(
        const std::unordered_map<std::string, std::string>& headers
    ) const {
        auto it = headers.find(ACCEPT_ENCODING);

        if (it != headers.end()) {
            auto requested_encodings = split(it->second, ',');

            for (const auto& encoding : requested_encodings) {
                if (std::ranges::find(supported_encodings_,
                                      encoding) != supported_encodings_.end()) {
                    return encoding; // Return first matching encoding
                }
            }
        }

        return "";
    }

    /**
     * @brief Compresses a string using zlib with gzip format
     * 
     * @param str String to compress
     * @param compressionLevel Compression level (Z_BEST_COMPRESSION by default)
     * @return std::string Compressed string
     * @throws std::runtime_error if compression fails
     */
    static std::string compressString(
        const std::string& str, 
        int compressionLevel = Z_BEST_COMPRESSION
    ) {
        z_stream zs = {}; // z_stream is zlib's control structure

        // Initialize deflate with gzip format (15 + 16)
        if (deflateInit2(&zs, compressionLevel, Z_DEFLATED, 
                         15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            throw std::runtime_error("deflateInit failed while compressing.");
        }

        zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
        zs.avail_in = static_cast<uInt>(str.size());

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
