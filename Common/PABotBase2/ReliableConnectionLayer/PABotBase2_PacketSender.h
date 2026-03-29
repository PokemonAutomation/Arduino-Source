/*  PABotBase2 Packet Sender
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ReliableConnectionLayer_PacketSender_H
#define PokemonAutomation_PABotBase2_ReliableConnectionLayer_PacketSender_H

#include "Common/Cpp/StreamConnections/PollingStreamConnections.h"
#include "PABotBase2_PacketProtocol.h"

#ifdef PABB2_SIZING_OVERRIDE
#include "PABotBase2_Config.h"
#else

//  Must be power-of-two. (max 64)
#ifndef PABB2_PacketSender_SLOTS
#define PABB2_PacketSender_SLOTS                64
#endif

//  Must fit into size_t.
#ifndef PABB2_PacketSender_BUFFER_SIZE
#define PABB2_PacketSender_BUFFER_SIZE          16384
#endif

#ifndef PABB2_PacketSender_RETRANSMIT_COUNTER
#define PABB2_PacketSender_RETRANSMIT_COUNTER   2
#endif

#endif

namespace PokemonAutomation{
namespace PABotBase2{




class PacketSender{
    static constexpr uint8_t SLOTS = PABB2_PacketSender_SLOTS;
    static constexpr uint8_t SLOTS_MASK = SLOTS - 1;
    static_assert((SLOTS & SLOTS_MASK) == 0, "Must be power-of-two.");

public:
    PacketSender(
        UnreliableStreamSender& connection,
        uint8_t max_packet_size
    );
    void reset();

    void set_max_packet_size(uint8_t max_packet_size){
        m_max_packet_size = max_packet_size;
    }

    void print(bool ascii) const;


public:
    uint8_t slots_used() const{
        return m_slot_tail - m_slot_head;
    }
    size_t buffer_used() const{
        return m_buffer_tail - m_buffer_head;
    }


public:
    //  Remove the packet corresponding to the specified seqnum from the queue.
    //  Returns true is successful, false if seqnum is not in the queue.
    bool remove(uint8_t seqnum);

    //
    //  Send a packet with the specified opcode and extra data after the header.
    //  Returns true if successful. (enters the queue)
    //
    bool send_packet(uint8_t opcode, uint8_t extra_bytes, const void* extra_data);

    //
    //  Send a single packet in two parts:
    //      1.  Reserve a buffer to fill with the packet.
    //      2.  Commit the packet to the queue.
    //
    //  On success, "reserve()" will return a pointer to the packet to be filled.
    //  All fields of the packet header are already filled and should not be
    //  modified by the user. The user should only populate the bytes that follow
    //  the header.
    //
    //  On fail, reserve will return a null pointer. No changes are made to the
    //  queue on failure. Failures will happen if there is insufficient space in
    //  the queue.
    //
    //  "commit()" will add the packet to the queue. It will add the CRC checksum
    //  so you don't need to do it yourself.
    //
    //  If "reserve()" succeeds, it must be followed by "commit()" before any other
    //  function can be called on this queue. It is undefined behavior to separate
    //  "reserve()" and "commit()" with another function on the same queue.
    //
    PacketHeader* reserve_packet(uint8_t opcode, uint8_t extra_bytes);
    void commit_packet(PacketHeader* packet);

    //
    //  Send the specified data on the data stream.
    //  Returns the # of bytes actually sent.
    //  Returning less than "bytes" indicates the queue is full.
    //
    size_t send_stream(const void* data, size_t bytes);

    //  Returns true if something was retransmitted.
    bool iterate_retransmits();


public:
    //
    //  Out-of-band messages that bypass the queue and go out as-is.
    //  These may be dropped.
    //
    void send_oob_packet_empty(uint8_t seqnum, uint8_t opcode);
    void send_oob_packet_u8(uint8_t seqnum, uint8_t opcode, uint8_t data);
    void send_oob_packet_u16(uint8_t seqnum, uint8_t opcode, const uint16_t& data);
    void send_oob_packet_u32(uint8_t seqnum, uint8_t opcode, const uint32_t& data);
    void send_oob_packet_data(
        uint8_t seqnum, uint8_t opcode,
        uint8_t bytes, const void* data
    );
    void send_oob_packet_u32_data(
        uint8_t seqnum, uint8_t opcode,
        const uint32_t& u32,
        uint8_t bytes, const void* data
    );


private:
    UnreliableStreamSender& m_connection;

    uint8_t m_max_packet_size;  //  0 = 256 bytes

    uint8_t m_slot_head;
    uint8_t m_slot_tail;

    //  A timer that increments each time "pabb2_PacketSender_iterate_retransmits()"
    //  is called and nothing is re-transmitted.
    uint8_t m_retransmit_seqnum;

    //  If non-zero, it indicates we are in the middle of sending stream bytes.
    //  The current packet must be finished before sending anything else.
//    uint8_t m_pending_stream;

    uint16_t m_stream_offset;

    //  (buffer_head == buffer_tail) is ambiguous between empty and full.
    //  However, queue empty also has (slot_head == slot_tail).
    size_t m_buffer_head;
    size_t m_buffer_tail;

    //  These store the offsets within the buffer where the packet starts.
    //  The values stored here have their bits flipped so that zero means invalid.
    size_t m_offsets[SLOTS];

    uint8_t m_buffer[PABB2_PacketSender_BUFFER_SIZE];
};










}
}
#endif
