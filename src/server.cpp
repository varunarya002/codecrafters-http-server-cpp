#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

struct HttpRequest {
  std::string method;
  std::string path;
};

HttpRequest extract_request_from_client(const int &client_fd, const int &server_fd) {
  constexpr char PATH_DELIMITER = '/', WHITESPACE_DELIMITER = ' ';
  std::string client_request(1024, '\0');
  long bytes_received = recv(client_fd, &client_request, client_request.size(), 0);

  if (bytes_received < 0) {
    perror("Receive failed!");
    close(client_fd);
    close(server_fd);
    exit(EXIT_FAILURE);
  }
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

void send_response_to_client(const int client_fd, const std::string& response) {
  send(client_fd, response.c_str(), response.size(), 0);
  std::cout << "Response" << response <<"sent successfully!\n";
}

std::string get_response_from_request(const HttpRequest& request) {
  std::string message = "OK";
  int status_code = 200;

  if (request.path.size() > 0) {
    status_code = 404;
    message = "Not Found";
  }

  std::cout << "REQ->" << request.path << std::endl;

  return "HTTP/1.1\t" + std::to_string(status_code) + "\t" + message + "\r\n\r\n";
}

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

  HttpRequest raw_data = extract_request_from_client(client_fd, server_fd);
  std::string response = get_response_from_request(raw_data);
  send_response_to_client(client_fd, response);

  close(server_fd);

  return 0;
}
