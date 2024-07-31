#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <ctime>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class TCPServer {
public:
    TCPServer(int port) : port(port), server_fd(0) {
        // Создание сокета
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Настройка адреса сервера
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        // Привязка сокета к адресу
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        // Начало прослушивания
        if (listen(server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
    }

    void start() {
        std::cout << "Server started on port " << port << std::endl;
        while (true) {
            int new_socket;
            int addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            std::thread client_thread(&TCPServer::handle_client, this, new_socket);
            client_thread.detach();
        }
    }

private:
    int port;
    int server_fd;
    struct sockaddr_in address;
    std::mutex log_mutex;

    void handle_client(int socket) {
        char buffer[1024] = {0};
        int valread = read(socket, buffer, 1024);
        if (valread > 0) {
            std::lock_guard<std::mutex> lock(log_mutex);
            std::ofstream log_file("log.txt", std::ios::app);
            if (log_file.is_open()) {
                log_file << buffer << std::endl;
                log_file.close();
            } else {
                std::cerr << "Failed to open log file" << std::endl;
            }
        } else {
            std::cerr << "Failed to read from socket" << std::endl;
        }
        close(socket);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = 0;
    try {
        port = std::stoi(argv[1]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid port number: " << argv[1] << std::endl;
        return 1;
    }

    TCPServer server(port);
    server.start();

    return 0;
}
