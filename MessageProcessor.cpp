#pragma once
#include "Handler.h"
#include <openssl/rand.h>


void socket_handler::process_messages ( ) {
    while ( 1 ) {
        std::unique_lock<std::mutex> lock ( m_action_lock );

        while ( m_actions.empty ( ) ) {
            m_action_cond.wait ( lock );
        }

        action a = m_actions.front ( );
        m_actions.pop ( );

        lock.unlock ( );

        if ( a.type == SUBSCRIBE ) {
            std::unique_lock<std::mutex> con_lock ( m_connection_lock );
          
            server::connection_ptr con = server.get_con_from_hdl ( a.hdl );



            BinaryPacket packet ( a.hdl );

            unsigned char key [ 32 ];
            RAND_bytes ( key, sizeof ( key ) );


            packet.buffer.resize ( 32 );

            std::memcpy ( ( void * ) packet.buffer.data ( ), key, 32 );


            packet.setType ( PacketType::PACKET_KEY );

            packet.send ( );

            auto c = new client ( a.hdl );
            c->remote_endpoint = con->get_remote_endpoint ( );
            c->encryption_key.resize ( 32 );

            std::memcpy ( ( char * ) c->encryption_key.data ( ), packet.buffer.data ( ), packet.buffer.size ( ) );


            m_clients.push_back ( c );
            std::cout << "New connection opened from host " << con->get_remote_endpoint ( ) << ": encryption key " << websocketpp::utility::to_hex ( c->encryption_key ) << std::endl;
        }
        else if ( a.type == UNSUBSCRIBE ) {
          //  std::unique_lock<std::mutex> con_lock ( m_connection_lock );
            //m_connections.erase ( a.hdl );
        }
        else if ( a.type == MESSAGE ) {
            std::unique_lock<std::mutex> con_lock ( m_connection_lock );

            auto client = this->get_client_by_connection ( a.hdl );
            
            BinaryPacket packet ( a.hdl );
            
            msgpack::unpacked unpacked_msg;
            msgpack::unpack ( unpacked_msg, a.msg->get_payload ( ).data ( ), a.msg->get_payload ( ).size ( ) );
            msgpack::object obj = unpacked_msg.get ( );
            
            obj.convert ( packet );
            
            if ( client && packet.encrypted ) {
            
                packet.decrypt ( ( unsigned char * ) client->encryption_key.data ( ) );
            
            }
            
            switch ( packet.getType ( ) ) {
            case PacketType::PACKET_LOGIN:
            {
            
                LoginPacket login;

                packet.get ( ).convert ( login );


                /*for ( auto & c : login.username )
                    c ^= 2;
                for ( auto & c : login.password )
                    c ^= 2;*/

                std::cout << "Login from  " << login.username << std::endl;

                client->Authentificated = true;



                auto res = LoginResult ( "Invalid username or password.", client->Authentificated, 32 );

                BinaryPacket loginResult(a.hdl);

                loginResult.setType ( PacketType::PACKET_LOGIN );

                loginResult.pack ( res );

                loginResult.send ( );
            }
            break;
            
            case PacketType::PACKET_REQUEST_INJECTION:
            {
                std::cout << "Recived PACKET_REQUEST_INJECTION from client " << std::endl;
            
                if ( client ) {
                    std::cout << "Recived from client : PACKET_REQUEST_INJECTION " << std::endl;
            
                    client->csgo_client = new ImageMap ( &cheatHandler );
            
                    std::uint64_t m_dwAllocationSize = client->csgo_client->GetAllocationSize ( );
            
            
                    BinaryPacket send_packet ( a.hdl );
            
            
                    send_packet.pack ( m_dwAllocationSize );
                    send_packet.setType ( PacketType::PACKET_ALLOC_SIZE );
            
                    send_packet.send ( );
                    std::cout << "Sent to client : PACKET_ALLOC_SIZE, size " << m_dwAllocationSize << std::endl;
            
                }
            }
            break;
            case PacketType::PACKET_ALLOC_ADDRESS:
            
            {
            
                if ( client ) {
            
            
            
                    msgpack::unpacked msg;
                    msgpack::unpack ( msg, packet.buffer.data ( ), packet.buffer.size ( ) );
            
                    std::uint64_t address;
            
                    msg.get ( ).convert ( address );
            
            
                    client->csgo_client->allocatedBuffer = address;
                    std::cout << "Recived from client : PACKET_ALLOC_ADDRESS " << address << std::endl;
            
            
                    std::cout << "Fixing relocs...: Allocated base " << client->csgo_client->allocatedBuffer << std::endl;
            
                    client->csgo_client->FixRelocs ( );
            
            
            
                    BinaryPacket importsPacket ( a.hdl );
            
                    std::stringstream ss;
                    msgpack::pack ( ss, cheatHandler.m_aImportList );
            
                    importsPacket.buffer = ss.str ( );
            
                    std::cout << "Sent import list to client. Size : " << cheatHandler.m_aImportList.size ( ) << std::endl;
            
                    // importsPacket.pack ( cheatHandler.m_aImportList );
            
                    importsPacket.setType ( PacketType::PACKET_IMPORTS_LIST );
            
                    importsPacket.send ( );
            
            
            
                }
            
            }
            break;
            case PacketType::PACKET_RECIVED_SECTIONS:
            {
                if ( client ) {
            
                    BinaryPacket MappedImage ( a.hdl );
            
                    MappedImage.buffer.resize ( client->csgo_client->cleanedBinary.size ( ) );
            
                    std::memcpy ( MappedImage.buffer.data ( ), client->csgo_client->cleanedBinary.data ( ), MappedImage.buffer.size ( ) );
            
                    // MappedImage.pack ( client->csgo_client->binary.data() );
                    MappedImage.setType ( PacketType::PACKET_MAPPED_IMAGE );
            
                    std::cout << "Sent PACKET_MAPPED_IMAGE size " << client->csgo_client->cleanedBinary.size ( ) << std::endl;
                    MappedImage.send ( );
            
                }
            }
            break;
            case PacketType::PACKET_IMPORTS_ADDRESSES:
            {
                if ( client ) {
            
                    msgpack::unpacked msg;
                    msgpack::unpack ( msg, packet.buffer.data ( ), packet.buffer.size ( ) );
            
            
                    msg.get ( ).convert ( client->csgo_client->clientImports );
            
                    std::cout << "Recived from client : PACKET_IMPORTS_ADDRESSES " << std::endl;
            
                    for ( auto & imp : client->csgo_client->clientImports )
                        std::cout << "Import address " << imp << std::endl;
            
                    std::cout << "Fixing imports..." << std::endl;
                    client->csgo_client->FixImports ( );
            
            
            
            
            
                    std::cout << "Sending sections image and entry point " << client->csgo_client->GetEntryPoint ( ) << std::endl;
            
                    PacketCPEData m_PEPacket;
                    m_PEPacket.m_dwEntry = client->csgo_client->GetEntryPoint ( );
                    LIMAGE_SECTION_HEADER * header = LIMAGE_FIRST_SECTION ( client->csgo_client->ntHd );
            
                    unsigned int nBytes = 0;
                    unsigned int virtualSize = 0;
                    unsigned int n = 0;
                    for ( unsigned int i = 0; client->csgo_client->ntHd->FileHeader.NumberOfSections; i++ ) {
                        //   Once we've reached the SizeOfImage, the rest of the sections
                        //   don't need to be mapped, if there are any.
                        if ( nBytes >= client->csgo_client->ntHd->OptionalHeader.SizeOfImage )
                            break;
            
                        /* if ( nBytes >= ntHd->OptionalHeader.SizeOfImage )
                             break;
            
            
            
                         a.push_back ( header->VirtualAddress );
                         b.push_back ( header->PointerToRawData );
                         c.push_back ( header->SizeOfRawData );
            
            
                         virtualSize = header->VirtualAddress;
                         header++;
                         virtualSize = header->VirtualAddress - virtualSize;
                         nBytes += virtualSize;*/
            
                        PacketCSection s;
                        s.m_iPtrToRaw = header->PointerToRawData;
                        s.m_iSizeOfRaw = header->SizeOfRawData;
                        s.m_iVirtualAddress = header->VirtualAddress;
            
                        m_PEPacket.m_aSections.push_back ( s );
            
                        virtualSize = header->VirtualAddress;
                        header++;
                        virtualSize = header->VirtualAddress - virtualSize;
                        nBytes += virtualSize;
            
                    }
            
            
            
                    BinaryPacket InjectionPacket ( a.hdl );
            
                    InjectionPacket.pack ( m_PEPacket );
                    InjectionPacket.setType ( PacketType::PACKET_SECTIONS );
            
                    std::cout << "Sent PACKET_SECTIONS m_PEPacket " << std::endl;
                    InjectionPacket.send ( );
            
            
            
                }
            }
            break;
            
            default:
                break;
            }
        }
        else {
            // undefined.
        }
    }
}

