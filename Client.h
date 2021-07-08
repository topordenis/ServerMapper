#pragma once
#include "MapperHandle.h"


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

	ImageMap * csgo_client;

	MSGPACK_DEFINE ( username, hwid );

};


class InjectPacket {
public:
	InjectPacket ( ) { };
		std::uint64_t m_dwEntry = 0x0;
		std::vector< std::uint8_t > buffer ;

		MSGPACK_DEFINE ( m_dwEntry, buffer );
};