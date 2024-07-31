#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

class TCPClient {
public:
    TCPClient(const std::string& name, int port, int period) : name(name), port(port), period(period) {}

    void start() {
        while (true) {
            int sock = 0;
            struct sockaddr_in serv_addr;
            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                std::cerr << "Socket creation error" << std::endl;
                return;
            }

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(port);

            if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
                std::cerr << "Invalid address/ Address not supported" << std::endl;
                return;
            }

            if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                std::cerr << "Connection Failed" << std::endl;
                return;
            }

            std::string message = get_current_time() + " " + name;
            send(sock, message.c_str(), message.length(), 0);
            close(sock);

            std::this_thread::sleep_for(std::chrono::seconds(period));
        }
    }

private:
    std::string name;
    int port;
    int period;

    std::string get_current_time() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "[%Y-%m-%d %X.") << std::setfill('0') << std::setw(3) << ms.count() << "]";
        return ss.str();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <name> <port> <period>" << std::endl;
        return 1;
    }

    std::string name = argv[1];
    int port = 0;
    int period = 0;

    try {
        port = std::stoi(argv[2]);
        period = std::stoi(argv[3]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid port or period number" << std::endl;
        return 1;
    }

    TCPClient client(name, port, period);
    client.start();

    return 0;
}
