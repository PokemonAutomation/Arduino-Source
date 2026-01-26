/*  PABotBase2 Reliable Stream Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdio.h>  //  REMOVE
#include "PABotbase2_ReliableStreamConnection.h"


void pabb2_ReliableStreamConnection_init(
    pabb2_ReliableStreamConnection* self,
    void* unreliable_connection,
    pabb2_fp_StreamSend unreliable_send,
    pabb2_fp_StreamRecv unreliable_recv
){
    self->unreliable_connection = unreliable_connection;
    self->unreliable_send = unreliable_send;
    self->unreliable_recv = unreliable_recv;

    pabb2_PacketSender_init(
        &self->reliable_sender,
        unreliable_connection,
        unreliable_send,
        PABB2_MAX_INCOMING_PACKET_SIZE
    );
    pabb2_PacketParser_init(&self->parser);
    pabb2_StreamCoalescer_init(&self->stream_coalescer);
}

void pabb2_ReliableStreamConnection_run_events(pabb2_ReliableStreamConnection* self){
    const pabb2_PacketHeader* packet = pabb2_PacketParser_pull_bytes(
        &self->parser,
        self->unreliable_connection, self->unreliable_recv
    );
    if (packet == NULL){
        pabb2_PacketSender_iterate_retransmits(&self->reliable_sender);
        return;
    }

    //  Check the packet status.
    switch (packet->magic_number){
    case PABB2_PacketParser_RESULT_VALID:
        break;
    case PABB2_PacketParser_RESULT_INVALID:
        printf("PABB2_PacketParser_RESULT_INVALID\n");
        pabb2_PacketSender_send_info(
            &self->reliable_sender,
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_INVALID_LENGTH
        );
        return;
    case PABB2_PacketParser_RESULT_CHECKSUM_FAIL:
        printf("PABB2_PacketParser_RESULT_CHECKSUM_FAIL\n");
        pabb2_PacketSender_send_info(
            &self->reliable_sender,
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_INVALID_CHECKSUM_FAIL
        );
        return;
    default:
        printf("Internal Error: Unrecognized packet state.\n");
        return;
    }

    //  Now handle the different opcodes.
    switch (packet->opcode){
    case PABB2_CONNECTION_OPCODE_ASK_RESET:
        pabb2_PacketSender_send_ack(
            &self->reliable_sender,
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_RESET
        );
        pabb2_PacketSender_reset(&self->reliable_sender);
        pabb2_PacketParser_reset(&self->parser);
        pabb2_StreamCoalescer_reset(&self->stream_coalescer);
        pabb2_StreamCoalescer_push_packet(&self->stream_coalescer, 0);
        return;
    case PABB2_CONNECTION_OPCODE_ASK_VERSION:
        pabb2_StreamCoalescer_push_packet(&self->stream_coalescer, packet->seqnum);
        pabb2_PacketSender_send_ack_u32(
            &self->reliable_sender,
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_VERSION,
            PABB2_CONNECTION_PROTOCOL_VERSION
        );
        return;
    case PABB2_CONNECTION_OPCODE_ASK_PACKET_SIZE:
        pabb2_StreamCoalescer_push_packet(&self->stream_coalescer, packet->seqnum);
        pabb2_PacketSender_send_ack_u16(
            &self->reliable_sender,
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_PACKET_SIZE,
            PABB2_MAX_INCOMING_PACKET_SIZE
        );
        return;
    case PABB2_CONNECTION_OPCODE_ASK_BUFFER_SLOTS:
        pabb2_StreamCoalescer_push_packet(&self->stream_coalescer, packet->seqnum);
        pabb2_PacketSender_send_ack_u8(
            &self->reliable_sender,
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_BUFFER_SLOTS,
            PABB2_StreamCoalescer_SLOTS
        );
        return;
    case PABB2_CONNECTION_OPCODE_ASK_BUFFER_BYTES:
        pabb2_StreamCoalescer_push_packet(&self->stream_coalescer, packet->seqnum);
        pabb2_PacketSender_send_ack_u16(
            &self->reliable_sender,
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_RET_BUFFER_BYTES,
            PABB2_StreamCoalescer_BUFFER_SIZE
        );
        return;
    case PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA:
//        printf("Device: PABB2_CONNECTION_OPCODE_ASK_STREAM_DATA\n");
        if (pabb2_StreamCoalescer_push_stream(&self->stream_coalescer, (const pabb2_PacketHeaderData*)packet)){
            printf("Device: Succeeded push.\n");
            pabb2_PacketSender_send_ack_u16(
                &self->reliable_sender,
                packet->seqnum,
                PABB2_CONNECTION_OPCODE_RET_STREAM_DATA,
                pabb2_StreamCoalescer_bytes_available(&self->stream_coalescer)
            );
        }else{
            printf("Device: Failed to push.\n");
        }
        fflush(stdout);
        return;
    default:
        pabb2_PacketSender_send_info(
            &self->reliable_sender,
            packet->seqnum,
            PABB2_CONNECTION_OPCODE_UNKNOWN_OPCODE
        );
    }
}

