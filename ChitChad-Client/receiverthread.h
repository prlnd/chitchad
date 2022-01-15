#ifndef RECEIVERTHREAD_H
#define RECEIVERTHREAD_H

#include "systhread.h"
#include <string_view>

class ReceiverThread : public SysThread
{
    Q_OBJECT

public:
    ReceiverThread(SOCKET socket);

public slots:
    virtual void run() override;

signals:
    void getUserList(std::string users, std::string channels);
    void getMessage(std::string username, std::string message);
    void getFile(std::string username, std::string filename, std::string content);
    void getChannelMessage(std::string username, std::string channel, std::string message);
    void getChannelFile(std::string username, std::string channel, std::string filename, std::string content);

private:
    enum DataProtocol {
        UserList,   // <length><type><users>
        Message,    // <length><type><from_user><message>
        File,       // <length><type><from_user><file>
        ChannelMessage,
        ChannelFile
    };

    SOCKET m_socket;
    static inline const int BUFLEN = 8192;
};

#endif // RECEIVERTHREAD_H
