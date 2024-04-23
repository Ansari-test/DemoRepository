#include <iostream>
#include <WinSock2.h>
#include <chrono>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

class UDPSender {
public:
    UDPSender(const std::string& ipAddress, int port) : ipAddress(ipAddress), port(port) {
        // Initialize Winsock with retry
        for (int attempt = 1; attempt <= maxStartupAttempts; ++attempt) {
            int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (result == 0) {
                break;
            } else {
                std::cerr << "WSAStartup failed (Attempt " << attempt << "): " << result << std::endl;
                if (attempt < maxStartupAttempts) {
                    std::this_thread::sleep_for(std::chrono::seconds(retryDelaySeconds));
                } else {
                    std::cerr << "Max attempts reached. Unable to initialize Winsock." << std::endl;
                    throw std::runtime_error("Failed to initialize Winsock");
                }
            }
        }
    }

    ~UDPSender() {
        WSACleanup();
    }

    void sendMessage(const std::string& message) {
        // Create a socket
        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Failed to create socket" << std::endl;
            throw std::runtime_error("Failed to create socket");
        }

        // Server address and port
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());

        // Send the message
        int bytesSent = sendto(sock, message.c_str(), message.length(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Failed to send message: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            throw std::runtime_error("Failed to send message");
        }

        std::cout << "Message sent successfully" << std::endl;
    }

private:
    static constexpr int maxStartupAttempts = 3;
    static constexpr int retryDelaySeconds = 1;

    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in serverAddr;
    std::string ipAddress;
    int port;
};

int main() {
    try {
        UDPSender sender("192.168.1.100", 12345);
        sender.sendMessage("Hello, UDP Server!");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
