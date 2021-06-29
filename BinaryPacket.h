#pragma once
#include <vector>
#include "PacketType.h"


class BinaryPacket {
public:
	BinaryPacket ( websocketpp::connection_hdl client_handle );
	~BinaryPacket ( );

private:

	PacketType type;
	std::vector<unsigned char> buffer;

	bool encrypted = false;


	websocketpp::connection_hdl handle;
public:
	void decrypt ( unsigned char * key );
	void encrypt ( unsigned char* key );

};

