#pragma once

// Messages the world server sends to a district server over TCP.
namespace WS2DS
{
	enum Headers
	{
		ResponseToInitial = '0',     // followed by one InitialResponseCodes byte
		AccountEntersDistrict = '1'  // followed by AccountEnterBody bytes
	};

	enum InitialResponseCodes
	{
		NotAllowed = '0',
		DistrictAlreadyExists1 = '1',
		DistrictAlreadyExists2 = '2',
		RegisterSuccess = '3',
		IDis0 = '4'
	};

	// uint32 account id (little-endian) + 16-byte XXTEA key + 20-byte handshake hash
	const int AccountEnterBody = 4 + 16 + 20;
}
