/*  PABotBase2 Reliable Stream Connection (FW)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2FW_ReliableStreamConnection_H
#define PokemonAutomation_PABotBase2FW_ReliableStreamConnection_H

#include "Common/Cpp/StreamConnections/PollingStreamConnections.h"
#include "PABotBase2_PacketSender.h"
#include "PABotBase2_PacketParser.h"
#include "PABotBase2_StreamCoalescer.h"

#ifndef PABB2_ReliableConnectionFW_POLL_MS
#define PABB2_ReliableConnectionFW_POLL_MS      50
#endif

#ifdef PABB2_ENABLE
#include "PabbTime.h"
#else
#include "Common/Cpp/Time.h"
#endif

namespace PokemonAutomation{
namespace PABotBase2{



class ReliableStreamConnectionFW final : public ReliableStreamConnectionPolling{
public:
    ReliableStreamConnectionFW(UnreliableStreamConnectionPolling& unreliable_connection);

    size_t packets_received() const{
        return m_packets_received;
    }
    bool has_unacked_sends() const{
        return m_reliable_sender.slots_used() != 0;
    }

public:
    virtual bool reliable_send_all_or_nothing(const void* data, size_t bytes) override;
    virtual void reliable_send(const void* data, size_t bytes) override;
    virtual size_t reliable_recv(void* data, size_t bytes) override{
        return m_stream_coalescer.read(data, bytes);
    }

    virtual bool reset_flag_set() const override{
        return m_reset_flag;
    }
    virtual void clear_reset_flag() override{
        m_reset_flag = false;
    }

    virtual bool run_events() override;
    virtual void wait_for_event(WallDuration timeout) override;


public:
    //  Send out-of-band messages.
    //  These are not part of the reliable protocol and may be dropped.

    void send_oob_info_u32(uint32_t data){
        m_reliable_sender.send_oob_packet_u32(0, PABB2_CONNECTION_OPCODE_INFO_U32, data);
    }
    void send_oob_info_binary(const void* data, uint8_t bytes);
    void send_oob_info_str(const char* str);
    void send_oob_info_label_h32(const char* str, const uint32_t& data){
        send_oob_info_label_i32(PABB2_CONNECTION_OPCODE_INFO_LABEL_H32, str, data);
    }
    void send_oob_info_label_u32(const char* str, const uint32_t& data){
        send_oob_info_label_i32(PABB2_CONNECTION_OPCODE_INFO_LABEL_U32, str, data);
    }
    void send_oob_info_label_i32(const char* str, const int32_t& data){
        send_oob_info_label_i32(PABB2_CONNECTION_OPCODE_INFO_LABEL_I32, str, data);
    }


public:
    //  For debugging
    const StreamCoalescer& stream_coalescer() const{
        return m_stream_coalescer;
    }


private:
    void send_oob_info_label_i32(uint8_t opcode, const char* str, uint32_t data);
    bool iterate_retransmits();


private:
    UnreliableStreamConnectionPolling& m_unreliable_connection;
    PacketSender m_reliable_sender;
    PacketParser m_parser;
    StreamCoalescer m_stream_coalescer;

    size_t m_packets_received = 0;

    WallClock m_last_retransmit;

    bool m_reset_flag = false;

    //  Don't allow any stream traffic until CC is ready.
    //  The MLC layer will get stuck in a bad state if we end up between packets.
    bool m_stream_ready = false;
};







}
}
#endif
