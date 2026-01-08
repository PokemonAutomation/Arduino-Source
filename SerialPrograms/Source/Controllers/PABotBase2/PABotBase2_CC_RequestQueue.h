/*  PABotBase2 CC Request Queue
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_CC_RequestQueue_H
#define PokemonAutomation_PABotBase2_CC_RequestQueue_H

#include <cstring>
#include <vector>
#include <map>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/PABotBase2/PABotBase2_Connection.h"

namespace PokemonAutomation{


struct StreamSender{
    virtual size_t send(const void* data, size_t bytes) = 0;
};
#if 0
struct StreamListener{
    virtual void recv(const void* data, size_t bytes) = 0;
};
#endif

namespace PABotBase2{


std::string dump_packet(const pabb2_PacketHeader* packet, bool ascii = false);




class RequestQueue{
public:
    RequestQueue(
        Logger& logger,
        StreamSender& sender,
        uint16_t max_packet_size,
        uint32_t receiver_buffer_size
    );

    void reset();

    //  Send the specified data to the stream.
    //
    //  Returns false if queue has reached max size.
    //
    //  This is strong exception safe:
    //    - If it throws, it's as if nothing happened.
    //    - If it returns, the data will have entered the queue, but it may not
    //      have been successfully sent. If it failed to send, it will be
    //      picked up on the next retransmit run.
    //
    bool send_data(const uint8_t* data, size_t bytes);

    void report_acked(uint32_t stream_offset);


public:
    //  Debugging

    bool send_str(const char* str){
        return send_data((const uint8_t*)str, strlen(str));
    }
    std::string dump_queue(bool ascii = false) const;


private:
    std::vector<uint8_t> make_data_packet(
        uint32_t stream_offset,
        const uint8_t* data, size_t bytes
    ) const;

    void send_data_small(const uint8_t* data, size_t bytes);
    void send_data_large(const uint8_t* data, size_t bytes);


private:
    Logger& m_logger;
    StreamSender& m_sender;

    size_t m_max_packet_size;
    size_t m_receiver_buffer_size;
    size_t m_max_data_per_packet;

    uint32_t m_stream_offset_head = 0;
    uint32_t m_stream_offset_tail = 0;

    struct Entry{
        size_t stream_bytes;
        std::vector<uint8_t> packet;
    };
    std::map<uint32_t, Entry> m_unacked_packets;
};







}
}
#endif

