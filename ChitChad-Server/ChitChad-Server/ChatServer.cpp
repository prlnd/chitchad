#include "ChatServer.h"
#include "ClientThread.h"
#include <iostream>

ChatServer::ChatServer()
{
	//---------------------------------------------
	// Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		std::cerr << "Error at WSAStartup\n";
		return;
	}
	//---------------------------------------------
	// Create a socket for sending data
	m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listenSocket == INVALID_SOCKET) {
		std::cerr << "Error at socket initialization with the following error code: " << WSAGetLastError() << '\n';
		return;
	}
	//---------------------------------------------
	// Set up the ServAddr structure with the IP address of
	// the receiver (server) (in this example case "192.168.113.85")
	// and the specified port number.
	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, ADDRESS, &servAddr.sin_addr);

	if (bind(m_listenSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		std::cerr << "bind() failed.\n";
		return;
	}
	//---------------------- 
	// Listen for incoming connection requests. 
	// on the created socket 
	if (listen(m_listenSocket, 1) == SOCKET_ERROR) {
		std::cerr << "Error at connection with the following error code: " << WSAGetLastError() << '\n';
		return;
	}
}

ChatServer::~ChatServer()
{
	//--------------------------------------------- 
	// When the application is finished sending, close the socket. 
	std::cerr << "Finished sending. Closing socket.\n";
	closesocket(m_listenSocket);
	//--------------------------------------------- 
	// Clean up and quit. 
	std::cerr << "Exiting.\n";
	WSACleanup();
}

void ChatServer::serve()
{
	//---------------------- 
	// Create a SOCKET for accepting incoming requests. 
	SOCKET acceptSocket;
	std::list<ClientThread> threads;
	CRITICAL_SECTION criticalSection;
	InitializeCriticalSection(&criticalSection);
	for (;;) {
		std::cerr << "Waiting for client to connect...\n";
		//---------------------- 
		// Accept the connection. 
		acceptSocket = accept(m_listenSocket, NULL, NULL);
		if (acceptSocket == INVALID_SOCKET) {
			std::cerr << "accept failed: " << WSAGetLastError() << '\n';
			return;
		}
		std::cerr << "Client connected.\n";
		threads.emplace_back(acceptSocket, threads, criticalSection);
		threads.back().start();
	}
}
