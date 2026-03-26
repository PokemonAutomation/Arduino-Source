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
inline PokemonAutomation::WallClock pabb_current_time(){
    return PokemonAutomation::current_time();
}
inline PokemonAutomation::Milliseconds pabb_milliseconds(int64_t milliseconds){
    return PokemonAutomation::Milliseconds(milliseconds);
}
#define pabb_time_wrapsafe_cmplt(x, y)  ((x) < (y))
#endif

namespace PokemonAutomation{
namespace PABotBase2{



class ReliableStreamConnectionFW final : public ReliableStreamConnectionPolling{
public:
    ReliableStreamConnectionFW(UnreliableStreamConnectionPolling& unreliable_connection);

    bool has_unacked_sends() const{
        return m_reliable_sender.slots_used() != 0;
    }

public:
    virtual void reliable_send(const void* data, size_t bytes) override{
        const char* ptr = (const char*)data;
        while (bytes > 0){
            size_t sent = m_reliable_sender.send_stream(ptr, bytes);
            ptr += sent;
            bytes -= sent;
        }
    }
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
    virtual void wait_for_event(uint16_t milliseconds) override;


public:
    void send_oob_info_u32(uint32_t data){
        m_reliable_sender.send_oob_packet_u32(0, PABB2_CONNECTION_OPCODE_INFO_U32, data);
    }
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

    WallClock m_last_retransmit;
    bool m_reset_flag = false;
};







}
}
#endif
