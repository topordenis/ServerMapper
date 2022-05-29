#pragma once
#include "MapperHandle.h"

class UserIdentity {
public:
	int id;
	std::string username;
	std::string hwid;
};

class client {
public:
	client( ){ }
	client ( connection_hdl _con ) {
		con = _con;
	}
	std::string remote_endpoint;


	std::string encryption_key;
	connection_hdl con;
	std::string username;
	std::string hwid;
	connection_hdl* mapper_connection;

	bool Authentificated = false;

	ImageMap * csgo_client;

	UserIdentity identity;
};


class InjectPacket {
public:
	InjectPacket ( ) { };
		std::uint64_t m_dwEntry = 0x0;
		std::vector< std::uint8_t > buffer ;

		MSGPACK_DEFINE ( m_dwEntry, buffer );
};