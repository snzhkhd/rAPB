#pragma once

#include <winsock2.h>

class UdpListener
{
private:
	SOCKET s;
	sockaddr_in si_me;
public:
	explicit UdpListener(unsigned short port);
	~UdpListener();
	bool IsValid() const;
	// Waits for one datagram. Returns its size (0 or more) or -1 on error.
	int Receive(char* buffer, int bufferSize, sockaddr_in* from);
};
