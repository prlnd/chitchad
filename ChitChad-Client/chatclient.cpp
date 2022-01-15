#include "chatclient.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <QDebug>
#include <limits>

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
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
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
    if (connect(clientSocket, (SOCKADDR*)&ServAddr, len) == SOCKET_ERROR) {
        std::cerr << "Error at connection with the following error code: " << WSAGetLastError() << '\n';
        return;
    }

    if (send(clientSocket, name.data(), name.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Error at sending name with the following error code: " << WSAGetLastError() << '\n';
        return;
    }
}

ChatClient::~ChatClient() {
    //---------------------------------------------
    // Shutdown connection
    std::cerr << "Shutting down the socket.\n";
    shutdown(clientSocket, SD_SEND);
    //---------------------------------------------
    // When the application is finished sending, close the socket.
    std::cerr << "Finished sending. Closing socket.\n";
    closesocket(clientSocket);
    //---------------------------------------------
    // Clean up and quit.
    std::cerr << "Exiting.\n";
    WSACleanup();
}

int ChatClient::getUsers()
{
    std::ostringstream oss;
    uint32_t size = sizeof(uint32_t) + sizeof(uint8_t);
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
            << static_cast<char>(DataProtocol::UserList);
    auto str = oss.str();
    return send(clientSocket, str.data(), str.size(), 0);
}

int ChatClient::sendUsername(std::string_view username)
{
    uint32_t size = username.size();
    std::ostringstream oss;
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
            .write(username.data(), size);
    auto str = oss.str();
    return send(clientSocket, str.data(), str.size(), 0);
}

int ChatClient::sendMessage(std::string username, std::string_view message)
{
    std::ostringstream oss;
    uint32_t size = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t) + username.length() + message.length();
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
            << static_cast<uint8_t>(DataProtocol::Message)
            << static_cast<uint8_t>(username.length())
            << username
            << message;
    auto str = oss.str();
    return sendChuncks(str);
}

int ChatClient::sendFile(std::string username, std::string_view path)
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
    oss.str(std::string());
    oss.clear();
    uint32_t size = sizeof(uint32_t) + sizeof(uint8_t)
            + sizeof(uint8_t) + username.length()
            + sizeof(uint8_t) + filename.length() + content.length();
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size));
    oss << static_cast<uint8_t>(DataProtocol::File)
        << static_cast<uint8_t>(username.length())
        << username
        << static_cast<uint8_t>(filename.length())
        << filename
        << content;
    auto str = oss.str();
    return sendChuncks(str);
}

int ChatClient::sendChannelMessage(std::string channel, std::string_view message)
{
    std::ostringstream oss;
    uint32_t size = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t) + channel.length() + message.length();
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
            << static_cast<uint8_t>(DataProtocol::ChannelMessage)
            << static_cast<uint8_t>(channel.length())
            << channel
            << message;
    auto str = oss.str();
    return sendChuncks(str);
}

int ChatClient::sendChannelFile(std::string channel, std::string_view path)
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
    oss.str(std::string());
    oss.clear();
    uint32_t size = sizeof(uint32_t) + sizeof(uint8_t)
            + sizeof(uint8_t) + channel.length()
            + sizeof(uint8_t) + filename.length() + content.length();
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size));
    oss << static_cast<uint8_t>(DataProtocol::ChannelFile)
        << static_cast<uint8_t>(channel.length())
        << channel
        << static_cast<uint8_t>(filename.length())
        << filename
        << content;
    auto str = oss.str();
    return sendChuncks(str);
}

int ChatClient::addChannel(std::string channel)
{
    uint32_t size = sizeof(uint32_t) + sizeof(uint8_t) + channel.size();
    std::ostringstream oss;
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
            << static_cast<uint8_t>(DataProtocol::AddChannel)
            << channel;
    auto str = oss.str();
    return send(clientSocket, str.data(), str.size(), 0);
}

int ChatClient::removeChannel(std::string channel)
{
    uint32_t size = sizeof(uint32_t) + sizeof(uint8_t) + channel.size();
    std::ostringstream oss;
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
            << static_cast<uint8_t>(DataProtocol::RemoveChannel)
            << channel;
    auto str = oss.str();
    return send(clientSocket, str.data(), str.size(), 0);
}

int ChatClient::addUser(std::string channel, std::string user)
{
    uint32_t size = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t) + channel.size() + user.size();
    std::ostringstream oss;
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
            << static_cast<uint8_t>(DataProtocol::AddUser)
            << static_cast<uint8_t>(channel.size())
            << channel
            << user;
    auto str = oss.str();
    return send(clientSocket, str.data(), str.size(), 0);
}

int ChatClient::removeUser(std::string channel, std::string user)
{
    uint32_t size = sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint8_t) + channel.size() + user.size();
    std::ostringstream oss;
    oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
            << static_cast<uint8_t>(DataProtocol::RemoveUser)
            << static_cast<uint8_t>(channel.size())
            << channel
            << user;
    auto str = oss.str();
    return send(clientSocket, str.data(), str.size(), 0);
}

int ChatClient::sendChuncks(std::string_view data)
{
    qDebug() << "Size:" << data.size() << "Data:" << QString::fromLatin1(data.data(), data.size());
    for (decltype(data.size()) i = 0; i < data.size(); ) {
        std::string_view substr = data.substr(i, i + BUFLEN);
        int iResult = send(clientSocket, substr.data(), substr.size(), 0);
        switch (iResult) {
        case SOCKET_ERROR: return SOCKET_ERROR;
        case 0: return i;
        default: i += iResult; break;
        }
    }
    return data.size();
}

std::string_view ChatClient::getFilename(std::string_view path)
{
    auto pos = path.find_last_of('/');
    auto filename = pos != path.npos ? path.substr(pos + 1u) : path;
    constexpr auto maxSize = std::numeric_limits<uint8_t>::max();
    return filename.size() <= maxSize
            ? filename
            : filename.substr(filename.size() - maxSize);
}
