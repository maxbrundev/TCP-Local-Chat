#pragma once

class Network
{
public:
	Network();
	~Network();

	void Bind();
	void Listen();
	void GetInfosFromClient(char* p_host, char* p_service);
	void Update();
private:
	void HandleClients();
	void Connect();
	void HandleChat(SOCKET& p_socketCopy, SOCKET& p_socketOut, char* p_bufferChat, char* p_host);
	void SendToClient(SOCKET& p_socketCopy, SOCKET& p_socketOut, char* p_bufferChat, char* p_host);

private:
	WSADATA m_wsa;
	SOCKET m_sock;
	SOCKADDR_IN m_address;
	SOCKET m_clientSock;
	fd_set m_master;

	unsigned int m_port;
	bool isRunning;
};