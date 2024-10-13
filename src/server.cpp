#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <utility>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <regex>
#include <unordered_map>

struct HttpRequest {
  std::string method;
  std::string path;
  std::string request_param;
};

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

    return request;
  }
private:
  const int client_fd;
  const int server_fd;
};

class AbstractUrlAction {
public:
  virtual ~AbstractUrlAction() = default;
  explicit AbstractUrlAction(const std::string &resource_name): resource_name(resource_name) {};
  [[nodiscard]] virtual std::string execute(const HttpRequest &http_request) const = 0;
protected:
  std::string resource_name;
};

class DefaultUrlAction : public AbstractUrlAction {
public:
  explicit DefaultUrlAction(const std::string &resource_name)
    : AbstractUrlAction(resource_name) {
  }

  [[nodiscard]] std::string execute(const HttpRequest &http_request) const override {
    const std::string message = "OK";
    HttpResponse http_response = HttpResponse(message, 200, "text/plain", 0, "");
    return http_response.sendResponse();
  }
};

class NotFoundUrlAction : public AbstractUrlAction {
public:
  explicit NotFoundUrlAction(const std::string &resource_name)
    : AbstractUrlAction(resource_name) {
  }

  [[nodiscard]] std::string execute(const HttpRequest &http_request) const override {
    const std::string message = "Not Found";
    HttpResponse http_response = HttpResponse(message, 404, "text/plain", 0, "");
    return http_response.sendResponse();
  }
};

class EchoUrlAction : public AbstractUrlAction {
public:
  explicit EchoUrlAction(const std::string &resource_name)
    : AbstractUrlAction(resource_name) {
  }

  [[nodiscard]] std::string execute(const HttpRequest &http_request) const override {
    const std::string message = "OK";
    HttpResponse http_response = HttpResponse(message, 200, "text/plain", http_request.request_param.length(), http_request.request_param);
    return http_response.sendResponse();
  }
};

class URLHandler {
public:
  URLHandler() = default;

  void registerUrl(const std::string& url_name, std::shared_ptr<AbstractUrlAction> action) {
    url_map[url_name] = std::move(action);
  }

  [[nodiscard]] std::string sendResponseForUrl(const HttpRequest &http_request) const {
    const std::string& url_path = http_request.path;

    for (auto &it: url_map) {
      std::string pattern = "^" + it.first + "(.*)";
      std::regex regex_pattern(pattern);
      std::smatch matches;
      if (std::regex_search(url_path, matches, regex_pattern)) {

        HttpRequest http_request_with_params = HttpRequest();
        http_request_with_params.method = http_request.method;
        http_request_with_params.path = http_request.path;
        std::string param = matches[1];
        if (param.starts_with("/")) {
          param = param.substr(1);
        }
        http_request_with_params.request_param = param;

        return it.second->execute(http_request_with_params);
      }
    }
    return NotFoundUrlAction("404").execute(http_request);
  }
private:
  std::pmr::unordered_map<std::string, std::shared_ptr<AbstractUrlAction>> url_map;
};

class Server {
public:
  Server(const int &client_fd, const int &server_fd, URLHandler& url_handler): client_fd(client_fd), server_fd(server_fd), url_handler(url_handler) {};
  void sendResponse() const {
    HttpRequestHandler request_handler = HttpRequestHandler(client_fd, server_fd);

    std::string response = url_handler.sendResponseForUrl(request_handler.parseRequest());
    send(client_fd, response.c_str(), response.size(), 0);

    std::cout << "Response sent successfully!\n";
  }
private:
  const int client_fd;
  const int server_fd;
  URLHandler& url_handler;
};

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  
  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage

  int server_fd = socket(AF_INET, SOCK_STREAM, 0); //AF->ADDRESS FAMILY; By specifying 0, you allow the system to choose the default protocol for the given socket type, which is TCP for SOCK_STREAM
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET; //AF->ADDRESS FAMILY
  server_addr.sin_addr.s_addr = INADDR_ANY; //allows the socket to bind to all available interfaces on the host machine. This means the server can accept connections on any network interface.
  server_addr.sin_port = htons(4221); // Set port. Function htons() (host-to-network short) converts the port number from host byte order to network byte order, which is necessary because network protocols use big-endian byte order.

  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";

  const int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  std::cout << "Client connected with fd: " << client_fd <<  std::endl;

  URLHandler url_handler = URLHandler();
  url_handler.registerUrl("", std::shared_ptr<AbstractUrlAction>(new DefaultUrlAction("")));
  url_handler.registerUrl("echo", std::shared_ptr<AbstractUrlAction>(new EchoUrlAction("echo")));

  const Server server = Server(client_fd, server_fd, url_handler);
  server.sendResponse();

  close(server_fd);

  return 0;
}
