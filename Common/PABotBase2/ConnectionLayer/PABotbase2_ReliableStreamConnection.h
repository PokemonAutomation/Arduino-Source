/*  PABotBase2 Reliable Stream Connection
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



class ReliableStreamConnectionFW{
public:
    ReliableStreamConnectionFW(StreamConnection& unreliable_connection);

    size_t send_stream(const void* data, size_t bytes){
        return m_reliable_sender.send_stream(data, bytes);
    }
    size_t read_stream(void* data, size_t bytes){
        return m_stream_coalescer.read(data, bytes);
    }

    void run_events();


public:
    //  For debugging
    const StreamCoalescer& stream_coalescer() const{
        return m_stream_coalescer;
    }


private:
    PacketSender m_reliable_sender;
    PacketParser m_parser;
    StreamCoalescer m_stream_coalescer;
};







}
}
#endif
