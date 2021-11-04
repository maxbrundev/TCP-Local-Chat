#pragma once

class Client
{
public:
	Client();
	~Client();
	void Update();

private:
	void InfoConnect();
	void Connect(std::string& p_address, int p_port);
	void Send();
	void Receive();

private:
	WSADATA m_wsa;
	SOCKET m_sock;
	SOCKADDR_IN m_address;
	struct hostent* m_hostInfos;
	std::thread m_threadReceive;

	int m_port;
	std::atomic<bool> isConnected;
	bool isFirstSend;
};
