#pragma once
#include <websocketpp/config/debug_asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <websocketpp/extensions/permessage_deflate/enabled.hpp>



using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;





#include "BinaryPacket.h"

#include "Client.h"
#include "PortableExecutable.h"
#include "MapperHandle.h"
#include <condition_variable>



struct deflate_config : public websocketpp::config::debug_core {
    typedef deflate_config type;
    typedef debug_core base;

    typedef base::concurrency_type concurrency_type;

    typedef base::request_type request_type;
    typedef base::response_type response_type;

    typedef base::message_type message_type;
    typedef base::con_msg_manager_type con_msg_manager_type;
    typedef base::endpoint_msg_manager_type endpoint_msg_manager_type;

    typedef base::alog_type alog_type;
    typedef base::elog_type elog_type;

    typedef base::rng_type rng_type;

    struct transport_config : public base::transport_config {
        typedef type::concurrency_type concurrency_type;
        typedef type::alog_type alog_type;
        typedef type::elog_type elog_type;
        typedef type::request_type request_type;
        typedef type::response_type response_type;
        typedef websocketpp::transport::asio::basic_socket::endpoint
            socket_type;
    };

    typedef websocketpp::transport::asio::endpoint<transport_config>
        transport_type;

    /// permessage_compress extension
    struct permessage_deflate_config { };

    typedef websocketpp::extensions::permessage_deflate::enabled
        <permessage_deflate_config> permessage_deflate_type;
};

typedef websocketpp::server<deflate_config> server;

typedef server::message_ptr message_ptr;

enum action_type {
    SUBSCRIBE,
    UNSUBSCRIBE,
    MESSAGE
};

struct action {
    action ( action_type t, connection_hdl h ) : type ( t ), hdl ( h ) { }
    action ( action_type t, connection_hdl h, server::message_ptr m )
        : type ( t ), hdl ( h ), msg ( m ) { }

    action_type type;
    websocketpp::connection_hdl hdl;
    server::message_ptr msg;
};


class socket_handler {
public:
    client * get_client_by_connection ( connection_hdl hdl );
    server::connection_ptr get_connection ( connection_hdl hdl );
    void listen ( );
    socket_handler ( );
	~socket_handler ( );
    void on_open ( connection_hdl hdl );
    void message_handle ( websocketpp::connection_hdl hdl, message_ptr msg );
     
    MapHandle cheatHandler;

    std::queue<action> m_actions;

    std::mutex m_action_lock;
    std::mutex m_connection_lock;
    std::condition_variable m_action_cond;

    void process_messages ( );
private:
	server server;
    std::vector<client*> m_clients;
	bool enable_logging = false;
  
};


extern std::unique_ptr< socket_handler > handler;


