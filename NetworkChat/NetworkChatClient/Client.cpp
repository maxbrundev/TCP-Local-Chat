#include "pch.h"

#include "Client.h"

Client::Client() : m_hostInfos(nullptr), m_port(8755), isConnected(false), isFirstSend(true)
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
		m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

Client::~Client()
{
	m_threadReceive.join();
	closesocket(m_sock);
	WSACleanup();
}

void Client::Update()
{
	InfoConnect();
	m_threadReceive = std::thread(&Client::Receive, this);
	Send();
}

void Client::InfoConnect()
{
	std::cout << "Enter IP Server:" << std::endl;
	std::string address;
	std::getline(std::cin, address);

	std::cout << "Enter Port Server:" << std::endl;
	std::string port;
	std::getline(std::cin, port);

	const int myPort = std::stoi(port);

	Connect(address, myPort);
}

void Client::Connect(std::string& p_address, int p_port)
{
	char serverAddress[15];
	memset(serverAddress, 0, sizeof(serverAddress));

	m_address.sin_family = AF_INET;
	m_address.sin_port = htons(p_port);

	inet_pton(AF_INET, p_address.c_str(), &m_address.sin_addr);

	std::cout << "Server Address: " << serverAddress << std::endl;

	if(connect(m_sock, reinterpret_cast<SOCKADDR*>(&m_address), sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		std::cout << "failed to connect, retry" << std::endl;
		InfoConnect();
	}
	else
	{
		isConnected.store(true);
	}
}

void Client::Send()
{
	std::string buffer;
	std::string pseudo;

	while(isConnected.load())
	{
		std::getline(std::cin, buffer);

		if(isFirstSend)
		{
			pseudo = buffer;
			isFirstSend = false;
		}
		else
		{
			std::ostringstream ss;
			ss << pseudo << ": " << buffer << std::endl;
			std::string strOut = ss.str();

			send(m_sock, strOut.c_str(), strOut.size() + 1, 0);

			std::cout << "sending: " << buffer << std::endl;
		}
	}
}

void Client::Receive()
{
	char bufferReceived[1024];
	
	while(isConnected.load())
	{
		memset(bufferReceived, 0, sizeof(bufferReceived));

		const int bytesReceived = recv(m_sock, bufferReceived, 1024, 0);

		if (bytesReceived <= 0)
		{
			isConnected.store(false);
		}

		std::cout << bufferReceived << std::endl;
	}

	std::cout << "Server Closed" << std::endl;
}