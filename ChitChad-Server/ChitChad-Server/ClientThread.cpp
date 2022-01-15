#include "ClientThread.h"
#include <iostream>
#include <string_view>
#include <sstream>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <set>

ClientThread::ClientThread(
	SOCKET socket,
	std::map<std::string, ClientThread>& threads,
	std::map<std::string, std::set<std::string>>& channels,
	CRITICAL_SECTION& criticalSection,
	std::string identifier
) : m_socket(socket),
	m_threads(threads),
	m_channels(channels),
	m_criticalSection(criticalSection),
	m_identifier(std::move(identifier))
{
}

ClientThread::~ClientThread()
{
	closesocket(m_socket);
}

void ClientThread::run(void)
{
	sendUsers();
	for (;;) {
		char buf[BUFLEN];
		int iResult = recv(m_socket, buf, sizeof(buf), 0);
		if (iResult == SOCKET_ERROR) {
			std::cerr << "Recv failed: " << WSAGetLastError() << '\n';
			return;
		} else if (iResult < sizeof(uint32_t) + sizeof(uint8_t)) {
			std::cerr << "Received buffer is too short\n";
			return;
		}
		std::string sbuf(buf, iResult);

		uint32_t bufLen = *reinterpret_cast<uint32_t*>(buf);
		for (int i = iResult; i < bufLen; i += iResult) {
			iResult = recv(m_socket, buf, sizeof(buf), 0);
			if (iResult == SOCKET_ERROR) {
				std::cerr << "Recv failed: " << WSAGetLastError() << '\n';
				return;
			}
			sbuf.append(buf, iResult);
		}

		auto recvBuf = sbuf.data();

		char type = recvBuf[sizeof(uint32_t)];
		std::cerr << "Received data (" << iResult << "): " << recvBuf << '\n';
		std::cerr << "Type " << (int)type << '\n';
		switch (type) {
		case ClientDataProtocol::UserList: {
			std::string sendBuf = getUsers();
			std::cerr << "Sending user and channel list: " << sendBuf << '\n';
			if (send(m_socket, sendBuf.data(), sendBuf.length(), 0) == SOCKET_ERROR) {
				std::cerr << "Send failed: " << WSAGetLastError() << '\n';
				return;
			}
			break;
		}
		case ClientDataProtocol::Message: {
			std::cerr << "Message\n";
			const char* pusername = recvBuf + sizeof(uint32_t) + sizeof(uint8_t);
			std::string username(pusername + sizeof(uint8_t), pusername[0]);
			const char* message = pusername + sizeof(uint8_t) + pusername[0];
			std::ostringstream oss;
			uint32_t msgLen = iResult - (message - recvBuf);
			uint32_t size = sizeof(uint32_t) + sizeof(uint8_t) + 1 + m_identifier.size() + msgLen;
			oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
				<< static_cast<char>(ServerDataProtocol::Message)
				<< static_cast<char>(m_identifier.size())
				<< m_identifier;
			oss.write(message, msgLen);
			auto str = oss.str();
			std::cerr << "Sending data: " << str << '\n';

			auto it = m_threads.find(username);
			if (it != m_threads.end()) {
				auto& [s, t] = *it;
				std::cerr << "Destination: " << s << '\n';
				if (send(t.m_socket, str.data(), str.length(), 0) == SOCKET_ERROR) {
					std::cerr << "Send failed: " << WSAGetLastError() << '\n';
					break;
				}
			}
			//EnterCriticalSection(&m_criticalSection);
			//for (auto& [s, t] : m_threads) {
			//	if (send(t.m_socket, str.data(), str.length(), 0) == SOCKET_ERROR) {
			//		std::cerr << "Send failed: " << WSAGetLastError() << '\n';
			//		break;
			//	}
			//}
			//LeaveCriticalSection(&m_criticalSection);
			break;
		}
		case ClientDataProtocol::File: {
			std::cerr << "File\n";
			const char* pusername = recvBuf + sizeof(uint32_t) + sizeof(uint8_t);
			std::string username(pusername + sizeof(uint8_t), pusername[0]);
			const char* message = pusername + sizeof(uint8_t) + pusername[0];
			std::ostringstream oss;
			uint32_t msgLen = iResult - (message - recvBuf);
			uint32_t size = sizeof(uint32_t) + sizeof(uint8_t) + 1 + m_identifier.size() + msgLen;
			oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
				<< static_cast<uint8_t>(ServerDataProtocol::File)
				<< static_cast<uint8_t>(m_identifier.size())
				<< m_identifier;
			oss.write(message, msgLen);
			auto str = oss.str();
			std::cerr << "Sending file: " << str << '\n';

			auto it = m_threads.find(username);
			if (it != m_threads.end()) {
				auto& [s, t] = *it;
				std::cerr << "Destination: " << s << '\n';
				if (send(t.m_socket, str.data(), str.length(), 0) == SOCKET_ERROR) {
					std::cerr << "Send failed: " << WSAGetLastError() << '\n';
					break;
				}
			}
			//EnterCriticalSection(&m_criticalSection);
			//for (auto& [s, t] : m_threads) {
			//	if (send(t.m_socket, str.data(), str.length(), 0) == SOCKET_ERROR) {
			//		std::cerr << "Send failed: " << WSAGetLastError() << '\n';
			//		break;
			//	}
			//}
			//LeaveCriticalSection(&m_criticalSection);
			break;
		}
		case ClientDataProtocol::ChannelMessage: {
			std::cerr << "Channel Message\n";
			const char* message = recvBuf + sizeof(uint32_t) + sizeof(uint8_t);
			std::string channel(message + sizeof(uint8_t), message[0]);
			std::ostringstream oss;
			uint32_t msgLen = iResult - (message - recvBuf);
			uint32_t size = sizeof(uint32_t) + sizeof(uint8_t) + 1 + m_identifier.size() + msgLen;
			oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
				<< static_cast<char>(ServerDataProtocol::ChannelMessage)
				<< static_cast<char>(m_identifier.size())
				<< m_identifier;
			oss.write(message, msgLen);
			auto str = oss.str();
			std::cerr << "Sending data: " << str << '\n';

			EnterCriticalSection(&m_criticalSection);
			if (channel == "<All>") {
				EnterCriticalSection(&m_criticalSection);
				for (auto& [s, t] : m_threads) {
					if (s != m_identifier && send(t.m_socket, str.data(), str.length(), 0) == SOCKET_ERROR) {
						std::cerr << "Send failed: " << WSAGetLastError() << '\n';
						break;
					}
				}
				LeaveCriticalSection(&m_criticalSection);
			} else {
				auto it = m_channels.find(channel);
				if (it != m_channels.end()) {
					auto& [ch, users] = *it;
					if (users.find(m_identifier) != users.end()) {
						for (auto& [s, t] : m_threads) {
							bool fnd = users.find(s) != users.end();
							if (s != m_identifier
								&& users.find(s) != users.end()
								&& send(t.m_socket, str.data(), str.length(), 0) == SOCKET_ERROR) {
								std::cerr << "Send failed: " << WSAGetLastError() << '\n';
								break;
							}
						}
					}
				}
			}
			LeaveCriticalSection(&m_criticalSection);
			break;
		}
		case ClientDataProtocol::ChannelFile: {
			std::cerr << "Channel File\n";
			const char* message = recvBuf + sizeof(uint32_t) + sizeof(uint8_t);
			std::string channel(message + sizeof(uint8_t), message[0]);
			std::ostringstream oss;
			uint32_t msgLen = iResult - (message - recvBuf);
			uint32_t size = sizeof(uint32_t) + sizeof(uint8_t) + 1 + m_identifier.size() + msgLen;
			oss.write(reinterpret_cast<const char*>(&size), sizeof(size))
				<< static_cast<uint8_t>(ServerDataProtocol::ChannelFile)
				<< static_cast<uint8_t>(m_identifier.size())
				<< m_identifier;
			oss.write(message, msgLen);
			auto str = oss.str();
			std::cerr << "Sending file: " << str << '\n';

			EnterCriticalSection(&m_criticalSection);
			if (channel == "<All>") {
				EnterCriticalSection(&m_criticalSection);
				for (auto& [s, t] : m_threads) {
					if (s != m_identifier && send(t.m_socket, str.data(), str.length(), 0) == SOCKET_ERROR) {
						std::cerr << "Send failed: " << WSAGetLastError() << '\n';
						break;
					}
				}
				LeaveCriticalSection(&m_criticalSection);
			} else {
				auto it = m_channels.find(channel);
				if (it != m_channels.end()) {
					auto& [ch, users] = *it;
					for (auto& [s, t] : m_threads) {
						if (s != m_identifier && users.find(s) != users.end() && send(t.m_socket, str.data(), str.length(), 0) == SOCKET_ERROR) {
							std::cerr << "Send failed: " << WSAGetLastError() << '\n';
							break;
						}
					}
				}
			}
			LeaveCriticalSection(&m_criticalSection);
			break;
		}
		case ClientDataProtocol::AddChannel: {
			std::cerr << "Add Channel\n";
			const char* pname = recvBuf + sizeof(uint32_t) + sizeof(uint8_t);
			uint32_t msgLen = iResult - (pname - recvBuf);
			std::string name(pname, msgLen);
			std::cerr << "Channel name: " << name << '\n';
			m_channels.emplace(name, std::set<std::string>({ m_identifier }));
			sendUsers();
			break;
		}
		case ClientDataProtocol::RemoveChannel: {
			const char* pname = recvBuf + sizeof(uint32_t) + sizeof(uint8_t);
			uint32_t msgLen = iResult - (pname - recvBuf);
			std::string name(pname, msgLen);
			if (name == "<All>")
				break;
			auto it = m_channels.find(name);
			if (it != m_channels.end() && it->second.find(m_identifier) != it->second.end()) {
				m_channels.erase(name);
				sendUsers();
			}
			break;
		}
		case ClientDataProtocol::AddUser: {
			const char* pchannel = recvBuf + sizeof(uint32_t) + sizeof(uint8_t);
			const char* pname = pchannel + pchannel[0] + sizeof(uint8_t);
			std::string channel(pchannel + sizeof(uint8_t), static_cast<uint32_t>(pchannel[0]));
			std::string name(pname, iResult - (pname - recvBuf));
			if (channel == "<All>")
				break;
			auto it = m_channels.find(channel);
			if (it != m_channels.end()) {
				auto& [ch, users] = *it;
				if (users.find(m_identifier) != users.end())
					users.insert(name);
			}
			break;
		}
		case ClientDataProtocol::RemoveUser: {
			const char* pchannel = recvBuf + sizeof(uint32_t) + sizeof(uint8_t);
			const char* pname = pchannel + pchannel[0] + sizeof(uint8_t);
			std::string channel(pchannel + sizeof(uint8_t), static_cast<uint32_t>(pchannel[0]));
			std::string name(pname, iResult - (pname - recvBuf));
			if (channel == "<All>")
				break;
			auto it = m_channels.find(channel);
			if (it != m_channels.end()) {
				auto& [ch, users] = *it;
				if (users.find(m_identifier) != users.end()) {
					users.erase(name);
					if (users.empty()) {
						m_channels.erase(it);
						sendUsers();
					}
				}
			}
			break;
		}
		default:
			std::cerr << "Unknown type " << (int)type << '\n';
			break;
		}
	}
}

std::string ClientThread::getUsers()
{
	std::string buffer(sizeof(uint32_t), '\0');
	buffer += static_cast<char>(ClientDataProtocol::UserList);
	buffer.append(sizeof(uint32_t), '\0');

	std::cerr << "Entering critical section\n";
	EnterCriticalSection(&m_criticalSection);
	for (auto it = m_threads.begin(); it != m_threads.end();) {
		auto& [s, t] = *it;
		if (t.isExited()) {
			std::cerr << "Client exited.\n";
			it = m_threads.erase(it);
		} else {
			buffer += static_cast<uint8_t>(s.size());
			buffer += s;
			++it;
		}
	}
	uint32_t size = buffer.size() - sizeof(uint32_t) * 2 - sizeof(uint8_t);
	*reinterpret_cast<uint32_t*>(buffer.data() + sizeof(uint32_t) + 1) = size;
	//buffer.replace(sizeof(uint32_t) + 1, sizeof(uint32_t) * 2 + 1, reinterpret_cast<const char*>(&size), sizeof(uint32_t));

	for (auto it = m_channels.begin(); it != m_channels.end(); ) {
		auto& [name, users] = *it;
		if (name != "<All>" && users.empty()) {
			it = m_channels.erase(it);
		} else {
			buffer += static_cast<uint8_t>(name.size());
			buffer += name;
			++it;
		}
	}
	LeaveCriticalSection(&m_criticalSection);
	std::cerr << "Leaving critical section\n";

	size = buffer.size();
	buffer.replace(0, sizeof(uint32_t), reinterpret_cast<const char*>(&size), sizeof(uint32_t));
	return buffer;
}

void ClientThread::sendUsers()
{
	std::string users = getUsers();
	std::cerr << "Sending user and channel list: " << users << '\n';

	std::cerr << "Entering critical section\n";
	EnterCriticalSection(&m_criticalSection);
	for (auto& [s, t] : m_threads) {
		int iResult = send(t.m_socket, users.data(), users.size(), 0);
		if (iResult == SOCKET_ERROR) {
			std::cerr << "Send failed: " << WSAGetLastError() << '\n';
			return;
		}
	}
	LeaveCriticalSection(&m_criticalSection);
	std::cerr << "Leaving critical section\n";
}
