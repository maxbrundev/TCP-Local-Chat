#include "pch.h"

#include "Network.h"

Network::Network() : m_port(8755), isRunning(false)
{
	try
	{
		int err = WSAStartup(MAKEWORD(2, 2), &m_wsa);
		if (err < 0)
		{
			throw std::runtime_error("WSAStartup failed!");
		}
	}
	catch (std::exception const& err)
	{
		std::cerr << err.what() << std::endl;
	}
	std::cout << "Init WSAS" << std::endl;

	try
	{
		m_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (m_sock == INVALID_SOCKET)
		{
			throw std::runtime_error("INVALID_SOCKET");
		}
	}
	catch (std::exception const& err)
	{
		std::cerr << err.what() << std::endl;
	}
	std::cout << "Init Socket" << std::endl;
}

Network::~Network()
{
	FD_CLR(m_sock, &m_master);
	closesocket(m_sock);
	WSACleanup();
}

void Network::Bind()
{
	m_address.sin_family = AF_INET;
	m_address.sin_port = htons(m_port);
	m_address.sin_addr.s_addr = htonl(INADDR_ANY);

	try
	{
		if (bind(m_sock, reinterpret_cast<SOCKADDR*>(&m_address), sizeof(m_address)) == SOCKET_ERROR)
		{
			throw std::runtime_error("INVALID_SOCKET");
		}
	}
	catch (std::exception const& err)
	{
		std::cerr << err.what() << std::endl;
	}
	std::cout << "Bind Socket" << std::endl;
}

void Network::Listen()
{
	try
	{
		if (listen(m_sock, SOMAXCONN) == SOCKET_ERROR)
		{
			throw std::runtime_error("INVALID_SOCKET");
		}
	}
	catch (std::exception const& err)
	{
		std::cerr << err.what() << std::endl;
	}
	std::cout << "Listen" << std::endl;

	isRunning = true;
}

void Network::GetInfosFromClient(char* p_host, char* p_service)
{
	SOCKADDR_IN clientAddress;
	int addressSize = sizeof(clientAddress);
	m_clientSock = accept(m_sock, reinterpret_cast<SOCKADDR*>(&clientAddress), &addressSize);

	/*if (getnameinfo(reinterpret_cast<SOCKADDR*>(&m_address), sizeof(m_address), p_host, NI_MAXHOST, p_service, NI_MAXSERV, 0) == 0)
	{
		std::cout << p_host << " Connected on Port [" << p_service << "]" << std::endl;
	}
		
	else
	{
		inet_ntop(AF_INET, &clientAddress.sin_addr, p_host, NI_MAXHOST);
		std::cout << p_host << " connected on port " << ntohs(clientAddress.sin_port) << std::endl;
	}*/

	inet_ntop(AF_INET, &clientAddress.sin_addr, p_host, NI_MAXHOST);
	std::cout << p_host << " connected on port " << ntohs(clientAddress.sin_port) << std::endl;
		
}

void Network::Update()
{
	Listen();
	HandleClients();
}

void Network::HandleClients()
{
	SOCKET socketCopy = 0;
	SOCKET socketOut = 0;

	char bufferChat[1024];
	memset(bufferChat, 0, sizeof(bufferChat));

	char host[NI_MAXHOST];
	char service[NI_MAXSERV];

	std::cout << "NETWORK INIT" << std::endl;

	FD_ZERO(&m_master);
	FD_SET(m_sock, &m_master);
	
	while (isRunning)
	{
		fd_set copy = m_master;
		const int socketCount = select(m_sock + 1, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++)
		{
			socketCopy = copy.fd_array[i];

			if (socketCopy == m_sock)
			{
				GetInfosFromClient(host, service);
				Connect();
			}
			else
			{
				HandleChat(socketCopy, socketOut, bufferChat, host);
			}
		}
	}

	std::string msg = "Server is shutting down. Goodbye\r\n";

	while (m_master.fd_count > 0)
	{
		SOCKET sock = m_master.fd_array[0];

		send(sock, msg.c_str(), msg.size() + 1, 0);

		FD_CLR(sock, &m_master);
		closesocket(sock);
	}

	closesocket(socketCopy);
	closesocket(socketOut);
}

void Network::Connect()
{
	FD_SET(m_clientSock, &m_master);

	std::string welcomeMsg = "Welcome to DofusChat Server! \n";
	send(m_clientSock, welcomeMsg.c_str(), welcomeMsg.size(), 0);

	std::string pseudoMsg = "Enter your Pseudo!\n";
	send(m_clientSock, pseudoMsg.c_str(), pseudoMsg.size(), 0);
}

void Network::HandleChat(SOCKET& p_socketCopy, SOCKET& p_socketOut, char* p_bufferChat, char* p_host)
{
	const int bytesReceived = recv(p_socketCopy, p_bufferChat, 1024, 0);

	if (bytesReceived <= 0)
	{
		closesocket(p_socketCopy);
		FD_CLR(p_socketCopy, &m_master);

		std::cout << p_host << " Disconnected" << std::endl;
	}
	else
	{
		SendToClient(p_socketCopy, p_socketOut, p_bufferChat, p_host);

		std::cout << p_host << ": " << p_bufferChat << std::endl;
		memset(p_bufferChat, 0, sizeof(p_bufferChat));
	}
}

void Network::SendToClient(SOCKET& p_socketCopy, SOCKET& p_socketOut, char* p_bufferChat, char* p_host)
{
	for (int i = 0; i < m_master.fd_count; i++)
	{
		p_socketOut = m_master.fd_array[i];

		if (p_socketOut != m_sock && p_socketOut != p_socketCopy)
		{
			std::ostringstream ss;
			ss << p_host << ": " << p_bufferChat << std::endl;
			std::string strOut = ss.str();
			send(p_socketOut, strOut.c_str(), strOut.size() + 1, 0);
		}
	}
}