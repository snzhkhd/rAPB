#pragma once

// A player the world server has announced to this district.
class Account
{
public:
	static const int KeySize = 16;
	static const int HandshakeHashSize = 20;

	Account(unsigned int id, const unsigned char* encryptionKey, const unsigned char* handshakeHash);
	unsigned int GetId() const;
	const unsigned char* GetEncryptionKey() const;
	const unsigned char* GetHandshakeHash() const;

private:
	unsigned int id;
	unsigned char encryptionKey[KeySize];
	unsigned char handshakeHash[HandshakeHashSize];
};
