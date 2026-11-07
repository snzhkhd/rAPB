#include "stdafx.h"

#include "Network.h"
#include "Account.h"
#include "Configuration.h"
#include "UdpListener.h"
#include "WS_DS_COM.h"

#include <mutex>
#include <cstring>

static const unsigned short DistrictUdpPort = 6969;

Configuration* cfg;
Network* world;
vector<Account*> accounts;
mutex accountsLock;

bool ProcessWorldMessage();
void UdpThread();

int main()
{
	Log_Clear();
	cfg = new Configuration("Configs\\District.conf");
	string token = GetTokenFromFile("Configs\\token.id");
	world = new Network();
	if (world->Setup(cfg->GetWorldIP(), atoi(cfg->GetWorldPort())) != OK)
	{
		Logger(lERROR, "Network::Setup()", "Socket setup failed");
		return 1;
	}
	Logger(lINFO, "Network::Setup()", "Ready to connect to World Server");
	if (world->Connect() != OK)
	{
		Logger(lERROR, "Network::Connect()", "Connection failed");
		return 1;
	}
	Logger(lSUCCESS, "Network::Connect()", "Connected to World Server");

	char port[8];
	sprintf_s(port, sizeof(port), "%d", DistrictUdpPort);
	if (world->SendInitial(cfg->GetDistrictType(), cfg->GetDistrictID(), cfg->GetDistrictLanguage(), GetPublicIP(), port, (char*)token.c_str()) != OK)
	{
		Logger(lERROR, "Network::Send()", "Data sending failed");
		return 1;
	}
	Logger(lINFO, "Network::Send()", "Initial data sent");

	// The first answer must be a successful registration, otherwise there is nothing to serve.
	char header[2];
	if (!world->ReceiveExact(header, 2) || header[0] != WS2DS::ResponseToInitial)
	{
		Logger(lERROR, "main()", "No registration answer from World Server");
		return 1;
	}
	switch (header[1])
	{
		case WS2DS::RegisterSuccess:
			Logger(lSUCCESS, "main()", "Registered at World Server");
			break;
		case WS2DS::NotAllowed:
			Logger(lERROR, "main()", "Not allowed to host a district (check token.id and CanHostDistrict)");
			return 1;
		case WS2DS::DistrictAlreadyExists1:
		case WS2DS::DistrictAlreadyExists2:
			Logger(lERROR, "main()", "District already exists");
			return 1;
		case WS2DS::IDis0:
			Logger(lERROR, "main()", "District ID can not be 0");
			return 1;
		default:
			Logger(lERROR, "main()", "Unknown registration answer '%c'", header[1]);
			return 1;
	}

	// One UDP socket serves every player in this district; packets are told apart by sender address.
	thread udp(UdpThread);
	udp.detach();

	while (ProcessWorldMessage()) {}
	Logger(lERROR, "main()", "Lost connection to World Server, exiting in 5 seconds...");
	Sleep(5000);
	return 1;
}

bool ProcessWorldMessage()
{
	char header;
	if (!world->ReceiveExact(&header, 1)) return false;

	if (header != WS2DS::AccountEntersDistrict)
	{
		Logger(lERROR, "ProcessWorldMessage()", "Unknown message from World Server: 0x%02X", (unsigned char)header);
		return false;
	}

	unsigned char body[WS2DS::AccountEnterBody];
	if (!world->ReceiveExact((char*)body, sizeof(body))) return false;

	unsigned int accountId = body[0] | (body[1] << 8) | (body[2] << 16) | ((unsigned int)body[3] << 24);
	Account* acc = new Account(accountId, body + 4, body + 4 + Account::KeySize);

	lock_guard<mutex> guard(accountsLock);
	for (size_t i = 0; i < accounts.size(); i++)
	{
		if (accounts[i]->GetId() == accountId)
		{
			// The player re-entered: keep only the newest keys.
			delete accounts[i];
			accounts.erase(accounts.begin() + i);
			break;
		}
	}
	accounts.push_back(acc);

	const unsigned char* k = acc->GetEncryptionKey();
	Logger(lINFO, "ProcessWorldMessage()", "District enter from account ID: %u (key: %02X%02X%02X%02X...)", accountId, k[0], k[1], k[2], k[3]);
	return true;
}

void UdpThread()
{
	UdpListener listener(DistrictUdpPort);
	if (!listener.IsValid()) return;
	Logger(lINFO, "UdpThread()", "Listening for clients on UDP port %d", DistrictUdpPort);

	char data[2048];
	sockaddr_in from;
	while (true)
	{
		int len = listener.Receive(data, sizeof(data), &from);
		if (len < 0)
		{
			Logger(lERROR, "UdpThread()", "recvfrom failed: %d", WSAGetLastError());
			continue;
		}
		// TODO: handshake, XXTEA decryption and the Unreal net protocol.
		Logger(lINFO, "UdpThread()", "Received %d bytes from %d.%d.%d.%d:%d", len,
			from.sin_addr.S_un.S_un_b.s_b1, from.sin_addr.S_un.S_un_b.s_b2,
			from.sin_addr.S_un.S_un_b.s_b3, from.sin_addr.S_un.S_un_b.s_b4, ntohs(from.sin_port));
	}
}
