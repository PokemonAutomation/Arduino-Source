/*  PABotBase2 Reliable Stream Connection (firmware-side)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ConnectionLayer_ReliableStreamConnection_H
#define PokemonAutomation_PABotBase2_ConnectionLayer_ReliableStreamConnection_H

#include "../PABotBase2_StreamInterface.h"
#include "PABotBase2_PacketSender.h"
#include "PABotBase2_PacketParser.h"
#include "PABotBase2_StreamCoalescer.h"

namespace PokemonAutomation{
namespace PABotBase2{



class ReliableStreamConnectionFW final : public StreamConnection{
public:
    ReliableStreamConnectionFW(StreamConnection& unreliable_connection);


public:
    virtual size_t send(const void* data, size_t bytes, bool is_retransmit = false) override{
        (void)is_retransmit;
        return m_reliable_sender.send_stream(data, bytes);
    }
    virtual size_t recv(void* data, size_t bytes) override{
        return m_stream_coalescer.read(data, bytes);
    }

    virtual bool reset_flag_set() const override{
        return m_reset_flag;
    }
    virtual void clear_reset_flag() override{
        m_reset_flag = false;
    }
    virtual bool run_events() override;


public:
    void send_oob_info_u32(uint32_t data){
        m_reliable_sender.send_oob_packet_u32(0, PABB2_CONNECTION_OPCODE_INFO_u32, data);
    }


public:
    //  For debugging
    const StreamCoalescer& stream_coalescer() const{
        return m_stream_coalescer;
    }


private:
    PacketSender m_reliable_sender;
    PacketParser m_parser;
    StreamCoalescer m_stream_coalescer;

    bool m_reset_flag = false;
};







}
}
#endif
