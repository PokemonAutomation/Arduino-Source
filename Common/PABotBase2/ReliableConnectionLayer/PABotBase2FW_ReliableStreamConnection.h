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


public:
    //  For debugging
    const StreamCoalescer& stream_coalescer() const{
        return m_stream_coalescer;
    }


private:
    UnreliableStreamConnectionPolling& m_unreliable_connection;
    PacketSender m_reliable_sender;
    PacketParser m_parser;
    StreamCoalescer m_stream_coalescer;

    bool m_reset_flag = false;
};







}
}
#endif
