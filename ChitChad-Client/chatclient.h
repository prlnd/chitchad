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
    int sendMessage(std::string username, std::string_view message);
    int sendFile(std::string username, std::string_view path);
    int sendChannelMessage(std::string channel, std::string_view message);
    int sendChannelFile(std::string channel, std::string_view path);
    int addChannel(std::string channel);
    int removeChannel(std::string channel);
    int addUser(std::string channel, std::string user);
    int removeUser(std::string channel, std::string user);
    int sendChuncks(std::string_view data);
    SOCKET clientSocket;

private:
    enum DataProtocol {
        UserList,
        Message,
        File,
        ChannelMessage,
        ChannelFile,
        AddChannel,
        RemoveChannel,
        AddUser,
        RemoveUser
    };

    std::string_view getFilename(std::string_view path);

    static inline const int BUFLEN = 8192;
};

#endif // CHATCLIENT_H
