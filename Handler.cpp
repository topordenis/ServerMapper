#include <iostream>


#include "Handler.h"


client * socket_handler::get_client_by_connection ( connection_hdl hdl ) {


    for ( auto & c : m_clients ) {
        if ( c->remote_endpoint == server.get_con_from_hdl ( hdl )->get_remote_endpoint ( ) ) {
            return c;
        }
    }
    return nullptr;
}

server::connection_ptr socket_handler::get_connection ( connection_hdl hdl ) {
    return server.get_con_from_hdl ( hdl );
}

void socket_handler::listen ( ) {
    server.listen ( 9002 );

    // Start the server accept loop
    server.start_accept ( );

    // Start the ASIO io_service run loop
    server.run ( );
    connection_hdl s;
  
    server::connection_ptr asd = server.get_con_from_hdl ( s );


    std::cout << "Server is now listening to incomming connections!\n";
}
socket_handler::socket_handler ( ) {
    try {
        // Set logging settings        
        if ( enable_logging  ) {
            server.set_access_channels ( websocketpp::log::alevel::all );
            server.set_error_channels ( websocketpp::log::elevel::all );
        }
        else {
            server.set_access_channels ( websocketpp::log::alevel::none );
            server.set_error_channels ( websocketpp::log::elevel::none );
        }

        // Initialize ASIO
        server.init_asio ( );

        server.set_open_handler ( bind ( &socket_handler::on_open, this, ::_1 ) );
        server.set_message_handler ( bind ( &socket_handler::message_handle, this, ::_1, ::_2 ) );
      
        std::cout << "Socket handler initialized!\n";
  
    

   

    }
    catch ( websocketpp::exception const & e ) {
        std::cout << e.what ( ) << std::endl;
    }
    catch ( const std::exception & e ) {
        std::cout << e.what ( ) << std::endl;
    }
    catch ( ... ) {
        std::cout << "other exception" << std::endl;
    }
}

socket_handler::~socket_handler ( ) {

}

std::unique_ptr< socket_handler > handler;