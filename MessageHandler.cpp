#pragma once
#include "Handler.h"
#include <openssl/rand.h>



void socket_handler::on_open ( connection_hdl hdl ) {

    server::connection_ptr con = server.get_con_from_hdl ( hdl );
    
   

    BinaryPacket packet(hdl);

    unsigned char key [ 32 ];
    RAND_bytes ( key, sizeof ( key ) );
  

    packet.buffer.resize ( 32 );

    std::memcpy ( (void*)packet.buffer.data ( ), key, 32 );

   
     packet.setType ( PacketType::PACKET_KEY );

    packet.send ( );

    auto c = new client ( hdl );
    c->remote_endpoint = con->get_remote_endpoint ( );
    c->encryption_key.resize ( 32 );

    std::memcpy ( (char*)c->encryption_key.data ( ), packet.buffer.data ( ), packet.buffer.size() );


    m_clients.push_back ( c );
    std::cout << "New connection opened from host " << con->get_remote_endpoint ( ) << ": encryption key " << websocketpp::utility::to_hex( c->encryption_key ) << std::endl;
}

// Define a callback to handle incoming messages
void socket_handler::message_handle ( websocketpp::connection_hdl hdl, message_ptr msg ) {
    /*std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message (" << msg->get_payload().size() << "): " << msg->get_payload()
              << std::endl;
    */
    try {
        if ( msg->get_opcode ( ) == websocketpp::frame::opcode::binary ) {
            std::cout << "Recived binary packet from client..." << std::endl;
            try {
                auto client = this->get_client_by_connection ( hdl );

                BinaryPacket packet(hdl);

                msgpack::unpacked unpacked_msg;
                msgpack::unpack ( unpacked_msg, msg->get_payload ( ).data ( ), msg->get_payload ( ).size ( ) );
                msgpack::object obj = unpacked_msg.get ( );

                obj.convert ( packet );

                if ( client && packet.encrypted ) {

                    packet.decrypt ( (unsigned char*)client->encryption_key.data() );

                }

                switch ( packet.getType ( ) ) {
                case PacketType::PACKET_KEY:

                    break;

                case PacketType::PACKET_REQUEST_INJECTION:
                {
                    std::cout << "Recived PACKET_REQUEST_INJECTION from client " << std::endl;
                    
                    if ( client ) {
                        std::cout << "Recived from client : PACKET_REQUEST_INJECTION " << std::endl;

                        client->csgo_client = new ImageMap ( &cheatHandler );

                        std::uint64_t m_dwAllocationSize = client->csgo_client->GetAllocationSize ( );

                        
                        BinaryPacket send_packet ( hdl );
                       
                    
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
                        std::cout << "Recived from client : PACKET_ALLOC_ADDRESS " << address  << std::endl;

                    
                        std::cout << "Fixing relocs...: Allocated base " << client->csgo_client->allocatedBuffer << std::endl;

                        client->csgo_client->FixRelocs ( );

                        
                       
                        BinaryPacket importsPacket(hdl);

                        std::stringstream ss;
                        msgpack::pack ( ss, cheatHandler.m_aImportList );
                        
                        importsPacket.buffer = ss.str ( );

                        std::cout << "Sent import list to client. Size : "  << cheatHandler.m_aImportList.size() << std::endl;
                       
                       // importsPacket.pack ( cheatHandler.m_aImportList );
                    
                        importsPacket.setType ( PacketType::PACKET_IMPORTS_LIST );
                        
                        importsPacket.send ( );
                     


                    }

                }
                    break;
                case PacketType::PACKET_RECIVED_SECTIONS:
                {
                    if ( client ) {

                        BinaryPacket MappedImage ( hdl );
                     
                        MappedImage.buffer.resize ( client->csgo_client->binary.size ( ) );
                        std::memcpy ( MappedImage.buffer.data ( ),  client->csgo_client->binary.data ( ), MappedImage.buffer.size ( ) );

                       // MappedImage.pack ( client->csgo_client->binary.data() );
                        MappedImage.setType ( PacketType::PACKET_MAPPED_IMAGE );

                        std::cout << "Sent PACKET_MAPPED_IMAGE size " << client->csgo_client->binary.size() << std::endl;
                        MappedImage.send ( );

                    }
                }
                break;
                case PacketType::PACKET_IMPORTS_ADDRESSES:
                {
                    if ( client ) {
                       
                        msgpack::unpacked msg;
                        msgpack::unpack ( msg, packet.buffer.data ( ), packet.buffer.size ( ) );

                        
                        msg.get().convert ( client->csgo_client->clientImports );

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



                        BinaryPacket InjectionPacket ( hdl );

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
            catch ( int err ) {

            }

        }
       
        //server.send ( hdl, msg->get_payload ( ), msg->get_opcode ( ) );
    }
    catch ( websocketpp::exception const & e ) {
        std::cout << "Echo failed because: "
            << "(" << e.what ( ) << ")" << std::endl;
    }
}
