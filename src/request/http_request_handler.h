#ifndef HTTP_REQUEST_HANDLER_H
#define HTTP_REQUEST_HANDLER_H

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <array>
#include <string>
#include <stdexcept>

#include "http_request.h"

/**
 * @class HttpRequestHandler
 * @brief Handles HTTP request parsing from socket data
 */
class HttpRequestHandler {
public:
  /**
   * @brief Constructs a new HTTP request handler
   * @param client_fd Client socket file descriptor
   * @param server_fd Server socket file descriptor
   */
  HttpRequestHandler(const int& client_fd, const int& server_fd)
    : client_fd_(client_fd), server_fd_(server_fd) {}

  /**
   * @brief Parses an HTTP request from the client socket
   * @return HttpRequest The parsed HTTP request object
   */
  [[nodiscard]] HttpRequest parseRequest() const {
    static constexpr size_t kMaxRequestSize = 1024;
    static constexpr char kPathDelimiter = '/';
    static constexpr char kWhitespaceDelimiter = ' ';
    static const std::string kCarriageDelimiter = "\r\n";
    
    std::array<char, kMaxRequestSize> buffer{};
    const long bytes_received = recv(client_fd_, buffer.data(), buffer.size(), 0);

    if (bytes_received < 0) {
      close(client_fd_);
      close(server_fd_);
      throw std::runtime_error("Failed to receive data from client");
    }

    HttpRequest request;
    if (bytes_received == 0) {
      return request; // Empty request
    }

    // Convert received bytes to string and ensure proper size
    std::string client_request(buffer.data(), bytes_received);
    
    // Parse HTTP method and path
    const size_t path_start_pos = client_request.find_first_of(kPathDelimiter);
    const size_t path_end_pos = client_request.find_first_of(kWhitespaceDelimiter, path_start_pos);
    
    request.method = client_request.substr(0, path_start_pos - 1);
    request.path = client_request.substr(path_start_pos + 1, path_end_pos - path_start_pos - 1);
    
    // Parse headers and body
    parseHeaders(request.headers, client_request);
    request.body = client_request.substr(client_request.find_last_of(kCarriageDelimiter) + 1);

    return request;
  }

private:
  const int client_fd_;
  const int server_fd_;

  /**
   * @brief Parses HTTP headers from the request string
   * @param headers Map to store the parsed headers
   * @param request_str Full HTTP request string
   */
  static void parseHeaders(std::unordered_map<std::string, std::string>& headers, 
                          const std::string& request_str) {
    static const std::string kCarriageDelimiter = "\r\n";
    static const std::string kKeyValueDelimiter = ": ";
    
    // Find headers section in the request
    const size_t header_start = request_str.find_first_of(kCarriageDelimiter);
    const size_t header_end = request_str.find_last_of(kCarriageDelimiter);
    
    if (header_start == std::string::npos || header_end == std::string::npos) {
      return;  // No headers found
    }
    
    // Extract headers text section
    const std::string headers_text = request_str.substr(
        header_start + 2, header_end - header_start - 3);
    
    // Parse individual headers
    size_t pos = 0;
    const size_t headers_end = headers_text.find_last_of(kCarriageDelimiter);
    
    while (pos < headers_end) {
      const size_t key_end = headers_text.find_first_of(kKeyValueDelimiter, pos);
      const size_t value_end = headers_text.find_first_of(kCarriageDelimiter, pos);
      
      if (key_end == std::string::npos || value_end == std::string::npos) {
        break;  // Malformed header
      }
      
      const std::string key = headers_text.substr(pos, key_end - pos);
      const std::string value = headers_text.substr(
          key_end + kKeyValueDelimiter.length(), 
          value_end - key_end - kKeyValueDelimiter.length());
          
      headers[key] = value;
      
      // Move to next header
      pos = value_end + kCarriageDelimiter.length();
    }
  }
};

#endif // HTTP_REQUEST_HANDLER_H
