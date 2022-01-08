#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <string_view>
#include <string>
#include "winsock2.h"

class ChatClient
{
public:
    ChatClient(std::string_view name, const char *address);
    ~ChatClient();
    int getUsers();
    int sendUsername(std::string_view username);
    int sendMessage(std::string_view message);
    int sendFile(std::string_view path);
    SOCKET ClientSocket;

private:
    std::string_view getFilename(std::string_view path);

    enum DataProtocol {
        UserList,
        Message,
        File
    };
};

#endif // CHATCLIENT_H
