#pragma once
#include <windows.h> 

class SysThread
{
public:
	SysThread(void);
	virtual ~SysThread();
	virtual bool start(void);
	virtual bool stop(unsigned int timeout = 0);
	inline volatile bool& isRunning(void)
	{
		return m_bRunning;
	}
	inline volatile bool& isExited(void)
	{
		return m_bExited;
	}
protected:
	virtual void run(void);

private:
	friend DWORD WINAPI runStub(LPVOID mthread);

public:
	static const unsigned int INFINIT_WAIT;

private:
	volatile bool m_bRunning;
	volatile bool m_bExited;
	HANDLE m_thread;
};
