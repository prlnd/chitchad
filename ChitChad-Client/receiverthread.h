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
    void getMessage(std::string_view username, std::string_view data);

private:
    enum DataProtocol {
        UserList,   // <length><type><users>
        Message,    // <length><type><from_user><message>
        File,       // <length><type><from_user><file>
        Success,    // <length><type><users>
        Failure     // <length><type><users>
    };

    SOCKET m_socket;
    static inline const int BUFLEN = 8192;
};

#endif // RECEIVERTHREAD_H
