/*  PABotBase2 Stream Coalescer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ConnectionLayer_StreamCoalescer_H
#define PokemonAutomation_PABotBase2_ConnectionLayer_StreamCoalescer_H

#include "PABotBase2_Connection.h"

#ifdef PABB2_SIZING_OVERRIDE
#include "PABotBase2_Config.h"
#else

//  Must be power-of-two. (max 64)
#ifndef PABB2_StreamCoalescer_SLOTS
#define PABB2_StreamCoalescer_SLOTS         64
#endif

//  Must be power-of-two, fits into uint16_t. (max 32768)
#ifndef PABB2_StreamCoalescer_BUFFER_SIZE
#define PABB2_StreamCoalescer_BUFFER_SIZE   16384
#endif

#endif

namespace PokemonAutomation{
namespace PABotBase2{



class StreamCoalescer{
    static const uint8_t SLOTS_MASK = (PABB2_StreamCoalescer_SLOTS) - 1;
    static const uint16_t BUFFER_MASK = (PABB2_StreamCoalescer_BUFFER_SIZE) - 1;

public:
    StreamCoalescer(){
        reset();
    }
    void reset();

    void print(bool ascii) const;


public:
    uint16_t free_bytes() const;

    void push_packet(uint8_t seqnum);

    //  Returns true if the packet has been received and can be acked.
    bool push_stream(const PacketHeaderData* packet);

    //
    //  Read data from the stream.
    //
    //  Returns the # of bytes actually read.
    //  Returns (size_t)-1 if the stream has been reset.
    //  Returning less than "max_bytes" indicates the stream is out of usable data.
    //
    size_t read(void* data, size_t max_bytes);


private:
    void write_buffer(
        const void* data,
        uint16_t stream_offset, uint8_t bytes
    );
    void read_buffer(
        void* data,
        uint16_t stream_offset, uint8_t bytes
    );
    void pop_leading_finished();


public:
    uint8_t m_slot_head;
    uint8_t m_slot_tail;

//    uint8_t m_seqnum;
    uint16_t m_stream_head;
    uint16_t m_stream_tail;

    //  0       =   Not received yet.
    //  0-254   =   Received stream packet. # is the size.
    //  255     =   Received non-stream packet.
    uint8_t m_lengths[PABB2_StreamCoalescer_SLOTS];

    uint16_t m_offsets[PABB2_StreamCoalescer_SLOTS];

    uint8_t m_buffer[PABB2_StreamCoalescer_BUFFER_SIZE];
};










}
}
#endif
