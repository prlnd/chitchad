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
        case DataProtocol::Message: {
            const char *pname = buf + sizeof(uint32_t) + 1;
            const char *pdata = pname + pname[0] + 1;
            std::string_view username(pname + 1, pname[0]);
            std::string_view message(pdata, iResult - (pdata - buf));
            qDebug() << "Emitting getMessage";
            emit getMessage(username, message);
            break;
        }
        default:
            qDebug() << "Unknown type " << buf[sizeof(uint32_t)];
            return;
        }
    }
}
