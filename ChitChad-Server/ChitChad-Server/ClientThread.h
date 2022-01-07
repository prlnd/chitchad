#pragma once
#include "SysThread.h"
#include <list>
#include <string>

class ClientThread : public SysThread
{
public:
    ClientThread(SOCKET socket, std::list<ClientThread>& threads, CRITICAL_SECTION& criticalSection);
    virtual ~ClientThread() override;
    virtual void run(void) override;

private:
    enum DataOffset {
        Length = sizeof(uint32_t),
        Type = 1
    };
    enum ClientDataProtocol {
        UserList,   // <length><type>
        Message,    // <length><type><to_user><message>
        File        // <length><type><to_user><file_name><file>
    };
    enum class ServerDataProtocol {
        UserList,   // <length><type><users>
        Message,    // <length><type><from_user><message>
        File,       // <length><type><from_user><file>
        Success,    // <length><type><users>
        Failure     // <length><type><users>
    };

    std::string getUsers();

    SOCKET m_socket;
    std::list<ClientThread>& m_threads;
    CRITICAL_SECTION& m_criticalSection;
    std::string m_identifier;
    static inline const int BUFLEN = 8192;
    static inline const int MAX_ID_LEN = 32;
};
