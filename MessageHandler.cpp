#pragma once
#include "Handler.h"


void socket_handler::on_open ( connection_hdl hdl ) {

    connection_ptr con = server.get_con_from_hdl ( hdl );

    m_clients.push_back ( new client ( hdl ) );

}

// Define a callback to handle incoming messages
void socket_handler::message_handle ( websocketpp::connection_hdl hdl, message_ptr msg ) {
    /*std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message (" << msg->get_payload().size() << "): " << msg->get_payload()
              << std::endl;
    */
    try {
        switch ( msg->get_opcode() == opcode ) {
        default:
            break;
        }
        hdl->s
       
        server.send ( hdl, msg->get_payload ( ), msg->get_opcode ( ) );
    }
    catch ( websocketpp::exception const & e ) {
        std::cout << "Echo failed because: "
            << "(" << e.what ( ) << ")" << std::endl;
    }
}
