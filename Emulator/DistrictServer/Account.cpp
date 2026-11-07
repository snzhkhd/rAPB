#include "stdafx.h"
#include "Account.h"

#include <cstring>

Account::Account(unsigned int id, const unsigned char* encryptionKey, const unsigned char* handshakeHash)
{
	this->id = id;
	// Keys and hashes are binary data that may contain zero bytes, so copy them by size.
	memcpy(this->encryptionKey, encryptionKey, KeySize);
	memcpy(this->handshakeHash, handshakeHash, HandshakeHashSize);
}

unsigned int Account::GetId() const
{
	return id;
}

const unsigned char* Account::GetEncryptionKey() const
{
	return encryptionKey;
}

const unsigned char* Account::GetHandshakeHash() const
{
	return handshakeHash;
}
