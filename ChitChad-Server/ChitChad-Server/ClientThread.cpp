#include "ClientThread.h"
#include <iostream>
#include <string_view>
#include <sstream>
#include <cstdint>
#include <algorithm>

ClientThread::ClientThread(SOCKET socket, std::list<ClientThread>& threads, CRITICAL_SECTION& criticalSection)
	: m_socket(socket)
	, m_threads(threads)
	, m_criticalSection(criticalSection)
{
}

ClientThread::~ClientThread()
{
	closesocket(m_socket);
}

void ClientThread::run(void)
{
	char recvBuf[BUFLEN];
	int iResult = recv(m_socket, recvBuf, BUFLEN, 0);
	if (iResult == SOCKET_ERROR) {
		std::cerr << "Recv failed: " << WSAGetLastError() << '\n';
		return;
	}
	m_identifier.assign(recvBuf, iResult <= MAX_ID_LEN ? iResult : MAX_ID_LEN);
	std::string_view bufView(recvBuf, iResult);
	std::cerr << "Received data: " << bufView << '\n';

	//std::string sendBuf(getUsers());
	//std::cerr << "Sending data to the client...\n";
	//if (send(m_socket, sendBuf.data(), sendBuf.length(), 0) == SOCKET_ERROR) {
	//	std::cerr << "Send failed: " << WSAGetLastError() << '\n';
	//	return;
	//}

	for (;;) {
		iResult = recv(m_socket, recvBuf, BUFLEN, 0);
		if (iResult == SOCKET_ERROR) {
			std::cerr << "Recv failed: " << WSAGetLastError() << '\n';
			return;
		} else if (iResult < DataOffset::Length + DataOffset::Type) {
			std::cerr << "Received buffer is too short\n";
			return;
		}
		std::string_view sv(recvBuf, iResult);
		char type = recvBuf[DataOffset::Length];
		std::cerr << "Received data: " << sv << ' ' << iResult << '\n';
		std::cerr << "Type " << (int)type << '\n';
		switch(type) {
		//case ClientDataProtocol::UserList:
		//	sendBuf = getUsers();
		//	if (send(m_socket, sendBuf.data(), sendBuf.length(), 0) == SOCKET_ERROR) {
		//		std::cerr << "Send failed: " << WSAGetLastError() << '\n';
		//		return;
		//	}
		//	break;
		case ClientDataProtocol::Message: {
			//const char* name = recvBuf + DataOffset::Length + DataOffset::Type;
			//const char* message = name + name[0] + 1;
			//std::string_view id(name + 1, name[0]);
			//auto it = std::find_if(m_threads.begin(), m_threads.end(), [id](ClientThread& t) {return t.m_identifier == id; });
			//if (it == m_threads.end()) {
			//	std::cerr << "User " << id << " not found\n";
			//	return;
			//}
			std::cerr << "Message\n";
			const char* message = recvBuf + DataOffset::Length + DataOffset::Type;
			std::ostringstream oss;
			uint32_t msgLen = iResult - (message - recvBuf);
			uint32_t size = DataOffset::Length + DataOffset::Type + 1 + m_identifier.size() + msgLen;
			(oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
				<< static_cast<char>(ServerDataProtocol::Message)
				<< static_cast<char>(m_identifier.size())
				<< m_identifier).write(message, msgLen);
			auto str = oss.str();
			std::cerr << "Sending data: " << str << '\n';
			//sendBuf.replace(0, DataOffset::Length, reinterpret_cast<const char*>(size), DataOffset::Length);
			//std::cerr << "Sending data: " << sendBuf << '\n';
			EnterCriticalSection(&m_criticalSection);
			for (auto& t : m_threads) {
				if (send(t.m_socket, str.data(), str.length(), 0) == SOCKET_ERROR) {
					std::cerr << "Send failed: " << WSAGetLastError() << '\n';
					break;
				}
			}
			LeaveCriticalSection(&m_criticalSection);
			break;
		}
		case ClientDataProtocol::File: {
			const char* message = recvBuf + DataOffset::Length + DataOffset::Type;
			std::ostringstream oss;
			uint32_t msgLen = iResult - (message - recvBuf);
			uint32_t size = DataOffset::Length + DataOffset::Type + 1 + m_identifier.size() + msgLen;
			(oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
				<< static_cast<char>(ServerDataProtocol::File)
				<< static_cast<char>(m_identifier.size())
				<< m_identifier).write(message, msgLen);
			auto str = oss.str();
			std::cerr << "Sending file: " << str.substr(20) << '\n';
			//sendBuf.replace(0, DataOffset::Length, reinterpret_cast<const char*>(size), DataOffset::Length);
			//std::cerr << "Sending data: " << sendBuf << '\n';
			EnterCriticalSection(&m_criticalSection);
			for (auto& t : m_threads) {
				if (send(t.m_socket, str.data(), str.length(), 0) == SOCKET_ERROR) {
					std::cerr << "Send failed: " << WSAGetLastError() << '\n';
					break;
				}
			}
			LeaveCriticalSection(&m_criticalSection);
			break;
		}
		default:
			std::cerr << "Unknown type " << (int)type << '\n';
			return;
		}
	}
}

std::string ClientThread::getUsers()
{
	std::string buffer(DataOffset::Length, '\0');
	buffer += static_cast<char>(ClientDataProtocol::UserList);

	std::cerr << "Entering critical section\n";
	EnterCriticalSection(&m_criticalSection);
	for (auto it = m_threads.begin(); it != m_threads.end();) {
		if (it->isExited()) {
			std::cerr << "Client exited.\n";
			it = m_threads.erase(it);
		} else {
			std::string_view id(it->m_identifier);
			char size = id.size();
			buffer += size;
			buffer += id.data();
			++it;
		}
	}
	LeaveCriticalSection(&m_criticalSection);

	uint32_t size = buffer.size();
	buffer.replace(0, DataOffset::Length, reinterpret_cast<const char*>(&size), DataOffset::Length);
	return buffer;
}
