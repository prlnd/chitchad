#include "receiverthread.h"
#include <QDebug>
#include "chadutils.h"

ReceiverThread::ReceiverThread(SOCKET socket) : m_socket(socket)
{
}

void ReceiverThread::run()
{
    for (;;) {
        char buf[BUFLEN];
        int iResult = recv(m_socket, buf, BUFLEN, 0);

        switch (iResult) {
        case SOCKET_ERROR:
            qDebug() << "Recv failed: " << WSAGetLastError();
            return;
        case 0:
            qDebug() << "Recv returned 0 with error: " << WSAGetLastError();
            return;
        default:
            if (iResult < static_cast<int>(sizeof(uint32_t) + sizeof(uint8_t))) {
                qDebug() << "Received buffer is too short\n";
                return;
            }
            break;
        }

        std::string sbuf(buf, iResult);
        int bufLen = static_cast<int>(*reinterpret_cast<uint32_t*>(buf));
        for (int i = iResult; i < bufLen; i += iResult) {
            iResult = recv(m_socket, buf, sizeof(buf), 0);
            if (iResult == SOCKET_ERROR) {
                qDebug() << "Recv failed: " << WSAGetLastError();
                return;
            }
            sbuf.append(buf, iResult);
        }

        auto recvBuf = sbuf.data();

        qDebug() << "Received:" << QString::fromStdString(sbuf);

        switch (recvBuf[sizeof(uint32_t)]) {
        case DataProtocol:: UserList: {
            const char *pusers = recvBuf + sizeof(uint32_t) + 1;
            uint32_t usersLength = *reinterpret_cast<const uint32_t*>(pusers);
            const char *pchannels = pusers + sizeof(uint32_t) + usersLength;
            std::string users(pusers + sizeof(uint32_t), usersLength);
            auto chlen = iResult - (pchannels - recvBuf);
            std::string channels(pchannels, chlen);
            qDebug() << "Emitting getUserList" << chad::stringViewToQString(users);
            emit getUserList(users, channels);
            break;
        }
        case DataProtocol::Message: {
            const char *pname = recvBuf + sizeof(uint32_t) + 1;
            const char *pdata = pname + pname[0] + 1;
            std::string username(pname + 1, pname[0]);
            std::string message(pdata, iResult - (pdata - recvBuf));
            qDebug() << "Emitting getMessage";
            emit getMessage(username, message);
            break;
        }
        case DataProtocol::File: {
            const char *pname = recvBuf + sizeof(uint32_t) + 1;
            const char *pfilename = pname + pname[0] + 1;
            const char *pcontent = pfilename + pfilename[0] + 1;
            std::string username(pname + 1, pname[0]);
            std::string filename(pfilename + 1, pfilename[0]);
            std::string content(pcontent, iResult - (pcontent - recvBuf));
            qDebug() << "Username:" << chad::stringViewToQString(username)
                     << "Filename:" << chad::stringViewToQString(filename)
                     << "Content:" << chad::stringViewToQString(content);
            qDebug() << "Emitting getFile";
            emit getFile(username, filename, content);
            break;
        }
        case DataProtocol::ChannelMessage: {
            const char *pname = recvBuf + sizeof(uint32_t) + 1;
            const char *pchannel = pname + pname[0] + 1;
            const char *pdata = pchannel + pchannel[0] + 1;
            std::string username(pname + 1, pname[0]);
            std::string channel(pchannel + 1, pchannel[0]);
            std::string message(pdata, iResult - (pdata - recvBuf));
            qDebug() << "Emitting getChannelMessage";
            emit getChannelMessage(username, channel, message);
            break;
        }
        case DataProtocol::ChannelFile: {
            const char *pname = recvBuf + sizeof(uint32_t) + 1;
            const char *pchannel = pname + pname[0] + 1;
            const char *pfilename = pchannel + pchannel[0] + 1;
            const char *pcontent = pfilename + pfilename[0] + 1;
            std::string username(pname + 1, pname[0]);
            std::string channel(pchannel + 1, pchannel[0]);
            std::string filename(pfilename + 1, pfilename[0]);
            std::string content(pcontent, iResult - (pcontent - recvBuf));
            qDebug() << "Username:" << chad::stringViewToQString(username)
                     << "Channel:" << chad::stringViewToQString(channel)
                     << "Filename:" << chad::stringViewToQString(filename)
                     << "Content:" << chad::stringViewToQString(content);
            qDebug() << "Emitting getChannelFile";
            emit getChannelFile(username, channel, filename, content);
            break;
        }
        default:
            qDebug() << "Unknown type " << static_cast<int>(recvBuf[sizeof(uint32_t)]);
            break;
        }
    }
}
