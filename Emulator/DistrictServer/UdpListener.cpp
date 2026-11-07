#include "stdafx.h"
#include "UdpListener.h"

#include <cstring>

UdpListener::UdpListener(unsigned short port)
{
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
	{
		Logger(lERROR, "UdpListener", "socket() failed: %d", WSAGetLastError());
		return;
	}

	memset(&si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(port);
	si_me.sin_addr.s_addr = INADDR_ANY;
	if (::bind(s, (sockaddr*)&si_me, sizeof(si_me)) == SOCKET_ERROR)
	{
		Logger(lERROR, "UdpListener", "bind() on UDP port %d failed: %d", port, WSAGetLastError());
		closesocket(s);
		s = INVALID_SOCKET;
	}
}

UdpListener::~UdpListener()
{
	if (s != INVALID_SOCKET) closesocket(s);
}

bool UdpListener::IsValid() const
{
	return s != INVALID_SOCKET;
}

int UdpListener::Receive(char* buffer, int bufferSize, sockaddr_in* from)
{
	int fromLen = sizeof(sockaddr_in);
	int received = recvfrom(s, buffer, bufferSize, 0, (sockaddr*)from, &fromLen);
	if (received == SOCKET_ERROR) return -1;
	return received;
}
