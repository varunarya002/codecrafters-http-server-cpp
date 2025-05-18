#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <regex>

#include "request/http_request_handler.h"
#include "url/abstract_url_action.h"
#include "url/default_url_action.h"
#include "url/echo_url_action.h"
#include "url/url_handler.h"
#include "url/user_agent_url_action.h"
#include "concurrent/thread_pool.h"
#include "url/file_url_action.h"

class Server {
public:
  Server(const int &client_fd, const int &server_fd, URLHandler& url_handler): client_fd(client_fd), server_fd(server_fd), url_handler(url_handler) {};
  void sendResponse(const std::string& directory_name) const {
    HttpRequestHandler request_handler(client_fd, server_fd);

    while (true) {
      // Parse incoming HTTP request
      HttpRequest request = request_handler.parseRequest();
      
      // Check if connection should be closed
      if (request.headers.empty()) {
        break;
      }

      // Process request and send response
      std::string response = url_handler.sendResponseForUrl(request, directory_name);
      size_t response_size = response.size();
      send(client_fd, response.c_str(), response_size, 0);

      if (request.headers.contains("Connection") && request.headers.at("Connection") == "close") {
        break;
      }
    }
    
    close(client_fd);
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
  std::string dir;
  
  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  if (argc == 3 && strcmp(argv[1], "--directory") == 0)
  {
    dir = argv[2];
  }

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

  URLHandler url_handler = URLHandler();
  url_handler.registerUrl("", std::shared_ptr<AbstractUrlAction>(new DefaultUrlAction("")));
  url_handler.registerUrl("echo", std::shared_ptr<AbstractUrlAction>(new EchoUrlAction("echo")));
  url_handler.registerUrl("user-agent", std::shared_ptr<AbstractUrlAction>(new UserAgentAction("user-agent")));
  url_handler.registerUrl("files", std::shared_ptr<AbstractUrlAction>(new FileUrlAction("files")));

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  ThreadPool pool(5);

  while (true) {
    std::cout << "Waiting for a client to connect...\n";

    const int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
    if (client_fd < 0) {
      std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
    }

    try {
      if (client_fd >= 0) {
        const Server server = Server(client_fd, server_fd, url_handler);
        pool.enqueue([server, dir] {server.sendResponse(dir);});
        std::cout << "Client connected with fd: " << client_fd <<  std::endl;
      }
    } catch (const std::exception& e) {
      std::cerr << "Error handling client: " << e.what() << std::endl;
      close(client_fd);
      close(server_fd);
      std::cout << "Server closed successfully!" << std::endl;
    }
  }

  return 0;
}
