#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

class ChatServer
{
public:
	ChatServer();
	~ChatServer();
	void serve();

private:
	SOCKET m_listenSocket;
	static inline const char* ADDRESS = "127.0.0.1";
	static inline const short PORT = 13000;
	static inline const int MAX_ID_LEN = 32;
};
