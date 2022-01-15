#pragma once
#include "SysThread.h"
#include <list>
#include <string>
#include <map>
#include <set>
#include <memory>

class ClientThread : public SysThread
{
public:
	ClientThread(
		SOCKET socket,
		std::map<std::string, ClientThread>& threads,
		std::map<std::string, std::set<std::string>>& channels,
		CRITICAL_SECTION& criticalSection,
		std::string identifier
	);
	virtual ~ClientThread() override;
	virtual void run(void) override;

private:
	enum ClientDataProtocol {
		UserList,   // <length><type>
		Message,    // <length><type><to_user><message>
		File,       // <length><type><to_user><file_name><file>
		ChannelMessage,
		ChannelFile,
		AddChannel,
		RemoveChannel,
		AddUser,
		RemoveUser
	};
	enum class ServerDataProtocol {
		UserList,   // <length><type><users>
		Message,    // <length><type><from_user><message>
		File,       // <length><type><from_user><file>
		ChannelMessage,
		ChannelFile
	};

	std::string getUsers();
	void sendUsers();

	SOCKET m_socket;
	std::map<std::string, ClientThread>& m_threads;
	std::map<std::string, std::set<std::string>>& m_channels;
	CRITICAL_SECTION& m_criticalSection;
	std::string m_identifier;
	static inline const int BUFLEN = 8192;
};
