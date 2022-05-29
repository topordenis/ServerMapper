#pragma once
#include "Handler.h"
#include <openssl/rand.h>



void socket_handler::on_open ( connection_hdl hdl ) {
    std::unique_lock< std::mutex> lock ( m_action_lock );
    //std::cout << "on_open" << std::endl;
    m_actions.push ( action ( SUBSCRIBE, hdl ) );
    lock.unlock ( );
    m_action_cond.notify_one ( );

    
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


                std::unique_lock<std::mutex> lock ( m_action_lock );
                //std::cout << "on_message" << std::endl;
                m_actions.push ( action ( MESSAGE, hdl, msg ) );
                lock.unlock ( );
                m_action_cond.notify_one ( );


        }
       
        //server.send ( hdl, msg->get_payload ( ), msg->get_opcode ( ) );
    }
    catch ( websocketpp::exception const & e ) {
        std::cout << "Echo failed because: "
            << "(" << e.what ( ) << ")" << std::endl;
    }
}
