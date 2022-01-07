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
    int sendMessage(std::string_view message/*, std::string_view username*/);
    int sendFile(std::string_view filename, std::string_view username);
    SOCKET ClientSocket;

private:
    enum DataProtocol {
        UserList,
        Message,
        File
    };
};

#endif // CHATCLIENT_H
