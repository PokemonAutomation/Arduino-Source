/*  PABotBase2 Reliable Stream Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ReliableStreamConnection_H
#define PokemonAutomation_PABotBase2_ReliableStreamConnection_H

#include "PABotBase2_StreamInterface.h"
#include "PABotBase2_PacketSender.h"
#include "PABotBase2_PacketParser.h"
#include "PABotBase2_StreamCoalescer.h"

#ifdef __cplusplus
extern "C" {
#endif



typedef struct{
    void* unreliable_connection;
    pabb2_fp_StreamSend unreliable_send;
    pabb2_fp_StreamRecv unreliable_recv;

    pabb2_PacketSender reliable_sender;
    pabb2_PacketParser parser;
    pabb2_StreamCoalescer stream_coalescer;
} pabb2_ReliableStreamConnection;



void pabb2_ReliableStreamConnection_init(
    pabb2_ReliableStreamConnection* self,
    void* unreliable_connection,
    pabb2_fp_StreamSend unreliable_send,
    pabb2_fp_StreamRecv unreliable_recv
);

inline size_t pabb2_ReliableStreamConnection_send_stream(
    pabb2_ReliableStreamConnection* self,
    const void* data, size_t bytes
){
    return pabb2_PacketSender_send_stream(&self->reliable_sender, data, bytes);
}
inline size_t pabb2_ReliableStreamConnection_read_stream(
    pabb2_ReliableStreamConnection* self,
    void* data, size_t bytes
){
    return pabb2_StreamCoalescer_read(&self->stream_coalescer, data, bytes);
}

void pabb2_ReliableStreamConnection_run_events(pabb2_ReliableStreamConnection* self);




#ifdef __cplusplus
}
#endif
#endif
