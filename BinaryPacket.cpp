#include "Handler.h"
#include <openssl/aes.h>
#include <openssl/rand.h>


BinaryPacket::BinaryPacket ( connection_hdl client_handle ) {
    handle = client_handle;
}

BinaryPacket::~BinaryPacket ( ) {

}

void BinaryPacket::encrypt ( unsigned char * key ) {

    AES_KEY enc_key;
    AES_set_encrypt_key ( key, 32, &enc_key );
    AES_cbc_encrypt ( ( unsigned char * ) this->buffer.data(), ( unsigned char * ) this->buffer.data ( ), this->buffer.size(), &enc_key, iv_enc, AES_ENCRYPT );

    this->encrypted = true;
}

void BinaryPacket::decrypt ( unsigned char * key ) {

    AES_KEY enc_key;
    AES_set_decrypt_key ( key, 32, &enc_key );
    AES_cbc_encrypt ( ( unsigned char * ) this->buffer.data ( ), ( unsigned char * ) this->buffer.data ( ), this->buffer.size ( ), &enc_key, iv_enc, AES_DECRYPT );

    this->encrypted = false;
}

