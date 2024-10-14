#ifndef HTTP_REQUEST_HANDLER_H
#define HTTP_REQUEST_HANDLER_H
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

#include "http_request.h"

class HttpRequestHandler {
public:
  HttpRequestHandler(const int &client_fd, const int &server_fd): client_fd(client_fd), server_fd(server_fd) {}
  [[nodiscard]] HttpRequest parseRequest() const {
    constexpr char PATH_DELIMITER = '/', WHITESPACE_DELIMITER = ' ';
    char req[1024] = {};
    long bytes_received = recv(client_fd, req, sizeof(req), 0);

    if (bytes_received < 0) {
      perror("Receive failed!");
      close(client_fd);
      close(server_fd);
      exit(EXIT_FAILURE);
    }

    std::string client_request(req);
    if (client_request.size() > bytes_received) {
      client_request.resize(bytes_received);
    }
    std::cout << "bytes received: " << bytes_received << std::endl;
    std::cout << "Client message length: " << client_request.size() << std::endl;

    HttpRequest request = HttpRequest();
    size_t path_start_pos = client_request.find_first_of(PATH_DELIMITER);
    size_t path_end_pos = client_request.find_first_of(WHITESPACE_DELIMITER, path_start_pos);

    request.method = client_request.substr(0, path_start_pos);
    request.path = client_request.substr(path_start_pos+1, path_end_pos - path_start_pos - 1);
    updateHeaderMap(request.headers, client_request);

    return request;
  }
private:
  const int client_fd;
  const int server_fd;

  static void convertToLowerCase(std::string& text) {
    for(auto& c : text)
    {
      c = tolower(c);
    }
  }

  static void updateHeaderMap(std::unordered_map<std::string, std::string>& headers, const std::string& client_request) {
    std::string CARRIAGE_DELIMITER = "\r\n", KEY_VALUE_DELIMITER = ": ";
    size_t header_start_pos = client_request.find_first_of(CARRIAGE_DELIMITER);
    size_t header_end_pos = client_request.find_last_of(CARRIAGE_DELIMITER);
    std::string headers_text = client_request.substr(header_start_pos+2, header_end_pos - header_start_pos - 3);

    header_start_pos = 0;
    header_end_pos = headers_text.find_last_of(CARRIAGE_DELIMITER);

    while (header_start_pos < header_end_pos) {
      size_t value_start_pos = headers_text.find_first_of(KEY_VALUE_DELIMITER, header_start_pos);
      size_t value_end_pos = headers_text.find_first_of(CARRIAGE_DELIMITER, header_start_pos);

      std::string key = headers_text.substr(header_start_pos, value_start_pos - header_start_pos);
      std::string value = headers_text.substr(value_start_pos+2, value_end_pos - value_start_pos - 2);
      convertToLowerCase(key);
      convertToLowerCase(value);
      headers[key] = value;

      header_start_pos = headers_text.find_first_of(CARRIAGE_DELIMITER, header_start_pos) + 2;
    }
  }
};

#endif //HTTP_REQUEST_HANDLER_H
