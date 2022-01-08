#include "chatclient.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <QDebug>

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

int ChatClient::sendMessage(std::string_view message)
{
    std::ostringstream oss;
    uint32_t size = sizeof(uint32_t) + 1 + message.length();
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size));
    oss << static_cast<char>(DataProtocol::Message)
        << message;
    auto str = oss.str();
    qDebug() << size << str.c_str();
    return send(ClientSocket, str.data(), str.size(), 0);
}

int ChatClient::sendFile(std::string_view path)
{
    std::ifstream ifs(path.data(), std::ifstream::binary);
    if (!ifs) {
        qDebug() << "Error while opening file";
        return -1;
    }
    std::ostringstream oss;
    auto filename = getFilename(path);
    oss << ifs.rdbuf();
    auto content = oss.str();
    oss.clear();
    uint32_t size = sizeof(uint32_t) + 1 + 1 + filename.length() + content.length();
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size));
    oss << static_cast<char>(DataProtocol::File)
        << static_cast<char>(filename.length())
        << filename
        << content;
    auto str = oss.str();
    return send(ClientSocket, str.data(), str.size(), 0);
}

std::string_view ChatClient::getFilename(std::string_view path)
{
    auto pos = path.find_last_of('/');
    return pos != path.npos ? path.substr(pos) : path;
}
