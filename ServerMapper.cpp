// ServerMapper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#pragma comment (lib, "crypt32")
#pragma comment (lib, "libssl.lib")
#pragma comment (lib, "zlibstat.lib")

#include <iostream>
#include "Handler.h"
#pragma once

void _except_handler4_common ( void ) { }
#include "CMapper.h"
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <Windows.h>
#include <TlHelp32.h>

// Target process name
#define PROCESS_NAME L"csgo.exe"

#define ERASE_ENTRY_POINT    TRUE
#define ERASE_PE_HEADER      TRUE
#define DECRYPT_DLL          FALSE

#define SUCCESS_MESSAGE      TRUE

void samp ( ) {

}

int main()
{
    std::cout << "Starting socket handler.\n";
    socket_handler handler;

    std::thread t ( bind ( &socket_handler::process_messages, &handler ) );

    handler.listen ( );



    return 0;
}

