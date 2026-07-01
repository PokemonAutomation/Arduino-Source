/*  PABotBase2 Stream Coalescer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ReliableConnectionLayer_StreamCoalescer_H
#define PokemonAutomation_PABotBase2_ReliableConnectionLayer_StreamCoalescer_H

#include "PABotBase2_PacketProtocol.h"

#ifdef PABB2_SIZING_OVERRIDE
#include "PABotBase2_Config.h"
#else

//  Must be power-of-two. (max 128)
#ifndef PABB2_StreamCoalescer_SLOT_CAPACITY
#define PABB2_StreamCoalescer_SLOT_CAPACITY      128
#endif

//  Must be power-of-two, fits into uint16_t. (max 32768)
#ifndef PABB2_StreamCoalescer_BUFFER_SIZE
#define PABB2_StreamCoalescer_BUFFER_SIZE   16384
#endif

#endif


#ifndef PABB2_StreamCoalescer_REORDER_WINDOW
#define PABB2_StreamCoalescer_REORDER_WINDOW     PABB2_StreamCoalescer_SLOT_CAPACITY
#endif


namespace PokemonAutomation{
namespace PABotBase2{



class StreamCoalescer{
    static constexpr uint8_t REORDER_WINDOW = PABB2_StreamCoalescer_REORDER_WINDOW;
    static constexpr uint8_t SLOT_CAPACITY = PABB2_StreamCoalescer_SLOT_CAPACITY;
    static constexpr uint8_t SLOT_MASK = SLOT_CAPACITY - 1;
    static constexpr uint16_t BUFFER_SIZE = PABB2_StreamCoalescer_BUFFER_SIZE;
    static constexpr uint16_t BUFFER_MASK = BUFFER_SIZE - 1;
    static_assert((SLOT_CAPACITY & SLOT_MASK) == 0, "Must be power-of-two.");
    static_assert((BUFFER_SIZE & BUFFER_MASK) == 0, "Must be power-of-two.");
    static_assert(REORDER_WINDOW <= SLOT_CAPACITY, "Reorder window cannot be larger than capacity.");


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
    //  Returning less than "max_bytes" indicates the stream is out of usable data.
    //
    size_t read(void* data, size_t max_bytes);


private:
    void write_buffer(
        const void* data,
        uint16_t stream_offset, uint8_t bytes
    );
    void read_buffer(
        void* data, //  Null means throw away the output.
        uint16_t stream_offset, uint16_t bytes
    ) const;
    void advance_slot_head();


public:
    //
    //  "tail" furthest into the future and marks where the next write should be.
    //
    //  "head" is the oldest hole in the coalescer. It may temporarily not be
    //  a hole, it just means it hasn't been advanced forward yet.
    //
    //  "free" is the oldest offset in the coalescer. Reads start here. It is
    //  named "free" because everything below this is empty and can be reused
    //  for new data.
    //
    //  Everything in the range [free, head) is valid data that hasn't been
    //  read yet.
    //
    uint8_t m_slot_head;
    uint8_t m_slot_tail;

    uint16_t m_stream_free;
    uint16_t m_stream_head;
    uint16_t m_stream_tail;

    //  0       =   Not received yet.
    //  0-254   =   Received stream packet. # is the size.
    //  255     =   Received non-stream packet.
    uint8_t m_lengths[SLOT_CAPACITY];

    //  0xffff = non-stream packet
    uint16_t m_end_offsets[SLOT_CAPACITY];

    uint8_t m_buffer[BUFFER_SIZE];
};










}
}
#endif
