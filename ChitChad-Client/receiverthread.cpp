#include "receiverthread.h"
#include <QDebug>

ReceiverThread::ReceiverThread(SOCKET socket) : m_socket(socket)
{

}

void ReceiverThread::run()
{
    char buf[BUFLEN];
    for (;;) {
        int iResult = recv(m_socket, buf, BUFLEN, 0);
        std::string s(buf, iResult);
        qDebug() << QString::fromStdString(s);

        if (iResult == SOCKET_ERROR) {
            qDebug() << "Recv failed: " << WSAGetLastError();
            return;
        } else if (iResult < sizeof(uint32_t) + 1) {
            qDebug() << "Received buffer is too short\n";
            return;
        }

        switch (buf[sizeof(uint32_t)]) {
        case DataProtocol:: UserList: {
            const char *pdata = buf + sizeof(uint32_t) + 1;
            std::string_view users(pdata, iResult - (pdata - buf));
            emit getUserList(users);
            break;
        }
        case DataProtocol::Message: {
            const char *pname = buf + sizeof(uint32_t) + 1;
            const char *pdata = pname + pname[0] + 1;
            std::string_view username(pname + 1, pname[0]);
            std::string_view message(pdata, iResult - (pdata - buf));
            qDebug() << "Emitting getMessage";
            emit getMessage(username, message);
            break;
        }
        case DataProtocol::File: {
            const char *pname = buf + sizeof(uint32_t) + 1;
            const char *pfilename = pname + pname[0] + 1;
            const char *pcontent = pfilename + pfilename[0] + 1;
            std::string_view username(pname + 1, pname[0]);
            std::string_view filename(pfilename + 1, pfilename[0]);
            std::string_view content(pcontent + 1, iResult - (pcontent - buf));
            qDebug() << "Emitting getFile";
            emit getFile(username, filename, content);
            break;
        }
        default:
            qDebug() << "Unknown type " << buf[sizeof(uint32_t)];
            return;
        }
    }
}
