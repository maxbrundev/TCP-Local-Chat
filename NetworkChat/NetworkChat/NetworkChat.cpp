#include "pch.h"

#include "Network.h"

int main()
{
	Network network;
	network.Bind();
	network.Update();

	std::cin.get();
	return EXIT_SUCCESS;
}