#include "chatclient.h"
#include <iostream>
#include <sstream>
#include <fstream>

ChatClient::ChatClient(std::string_view name, const char *address)
{
    WSADATA wsaData;
    sockaddr_in ServAddr;
    int Port = 13000;
    int len = sizeof(ServAddr);
    //---------------------------------------------
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) < 0) {
        std::cerr << "Error at WSAStartup\n";
        return;
    }
    //---------------------------------------------
    // Create a socket for sending data
    ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ClientSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket initialization with the following error code: " << WSAGetLastError() << '\n';
        return;
    }
    //---------------------------------------------
    // Set up the ServAddr structure with the IP address of
    // the receiver (server) (in this example case "192.168.113.85")
    // and the specified port number.
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_port = htons(Port);
    ServAddr.sin_addr.s_addr = inet_addr(address);
    //inet_pton(AF_INET, "127.0.0.1", &ServAddr.sin_addr);
    //---------------------------------------------

    std::cerr << "Building connection...\n";
    if (connect(ClientSocket, (SOCKADDR*)&ServAddr, len) == SOCKET_ERROR) {
        std::cerr << "Error at connection with the following error code: " << WSAGetLastError() << '\n';
        return;
    }

    if (send(ClientSocket, name.data(), name.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Error at sending name with the following error code: " << WSAGetLastError() << '\n';
        return;
    }
}

ChatClient::~ChatClient() {
    //---------------------------------------------
    // Shutdown connection
    std::cerr << "Shutting down the socket.\n";
    shutdown(ClientSocket, SD_SEND);
    //---------------------------------------------
    // When the application is finished sending, close the socket.
    std::cerr << "Finished sending. Closing socket.\n";
    closesocket(ClientSocket);
    //---------------------------------------------
    // Clean up and quit.
    std::cerr << "Exiting.\n";
    WSACleanup();
}

int ChatClient::getUsers()
{
    char buf[sizeof(uint32_t) + 1];
    buf[sizeof(uint32_t)] = DataProtocol::UserList;
    const uint32_t size = sizeof(buf);
    memcpy(buf, &size, sizeof(size));
    return send(ClientSocket, buf, size, 0);
}

int ChatClient::sendUsername(std::string_view username)
{
    uint32_t size = username.size();
    std::ostringstream ss;
    ss.write(reinterpret_cast<const char*>(&size), sizeof(size))
            .write(username.data(), size);
    auto str = ss.str();
    return send(ClientSocket, str.data(), str.size(), 0);
}

int ChatClient::sendMessage(std::string_view message/*, std::string_view username*/)
{
    std::ostringstream oss;
    uint32_t size = sizeof(uint32_t) + 1 + /*1 + username.length() +*/ message.length();
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size));
    oss << static_cast<char>(DataProtocol::Message)
//        << static_cast<char>(username.length())
//        << username
        << message;
    auto str = oss.str();
    return send(ClientSocket, str.data(), str.size(), 0);
}

int ChatClient::sendFile(std::string_view filename, std::string_view username)
{
    std::ifstream ifs(filename.data(), std::ifstream::binary);
    std::ostringstream oss;
    oss << ifs.rdbuf();
    auto fileContent = oss.str();
    uint32_t size = sizeof(uint32_t) + 1 + 1 + username.length() + 1 + filename.length() + fileContent.length();
    oss.clear();
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size));
    oss << static_cast<char>(DataProtocol::File)
        << static_cast<char>(username.length())
        << username
        << static_cast<char>(filename.length())
        << filename
        << fileContent;
    auto str = oss.str();
    return send(ClientSocket, str.data(), str.size(), 0);
}
