/*  PABotBase2 Packet Parser
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ConnectionLayer_PacketParser_H
#define PokemonAutomation_PABotBase2_ConnectionLayer_PacketParser_H

#include "../PABotBase2_StreamInterface.h"
#include "PABotBase2_Connection.h"

#ifdef PABB2_SIZING_OVERRIDE
#include "PABotBase2_Config.h"
#else

//  Maximum size of incoming packet + overhead.
//  Min Size: sizeof(largest header) + sizeof(uint32_t) + 1
//  Max Size: 256
#ifndef PABB2_MAX_INCOMING_PACKET_SIZE
#define PABB2_MAX_INCOMING_PACKET_SIZE      ((uint16_t)128)
#endif

#endif

namespace PokemonAutomation{
namespace PABotBase2{




#define PABB2_PacketParser_RESULT_VALID             0
#define PABB2_PacketParser_RESULT_INVALID           1
#define PABB2_PacketParser_RESULT_CHECKSUM_FAIL     2


struct PacketRunner{
    virtual void on_packet(const PacketHeader* data) = 0;
};


struct PacketParser{
public:
    PacketParser(StreamConnection& unreliable_connection)
        : m_unreliable_connection(unreliable_connection)
        , m_index(0)
    {}
    void reset(){
        m_index = 0;
    }


public:
    //
    //  Parse packets by consuming data by pulling from a reader.
    //
    //  Returns NULL if more data is needed.
    //  Returns the packet header on a completed/invalid packet.
    //  (return_value->magic_number) indicates the status of the packet.
    //
    //  This will never read more than is necessary unless there
    //  is an error.
    //
    const PacketHeader* pull_bytes();

    //
    //  Parse packets by consuming data from an existing buffer.
    //  This will consume all the bytes you give it and may call
    //  "packet_runner" multiple times.
    //
    void push_bytes(
        PacketRunner& packet_runner,
        const uint8_t* data, size_t bytes
    );


private:
    StreamConnection& m_unreliable_connection;

    uint8_t m_index;
    uint8_t m_buffer[PABB2_MAX_INCOMING_PACKET_SIZE];
};






}
}
#endif
