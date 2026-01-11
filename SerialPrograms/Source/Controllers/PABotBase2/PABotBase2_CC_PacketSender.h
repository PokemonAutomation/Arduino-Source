/*  PABotBase2 Packet Sender
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_CC_PacketSender_H
#define PokemonAutomation_PABotBase2_CC_PacketSender_H

#include <mutex>
#include <condition_variable>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/StreamInterface.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "Common/PABotBase2/PABotBase2_PacketSender.h"

namespace PokemonAutomation{
namespace PABotBase2{



class PacketSender{
public:
    PacketSender(
        Logger& logger,
        StreamSender& sender,
        WallDuration retransmit_timeout = Milliseconds(100)
    );
    ~PacketSender();

    bool send_packet(uint8_t opcode){
        return send_packet(opcode, 0, nullptr);
    }
    bool send_packet(uint8_t opcode, uint8_t extra_bytes, const void* extra_data);
    size_t send_stream(const uint8_t* data, size_t bytes);

    void report_acked(uint8_t seqnum);


private:
    static void sender_fp(void* context, const void* data, size_t bytes){
        ((StreamSender*)context)->send(data, bytes);
    }
    void retransmit_thread();


private:
    Logger& m_logger;
    StreamSender& m_sender;
    const WallDuration m_retransmit_timeout;

    pabb2_PacketSender m_queue;

    mutable std::mutex m_lock;
    std::condition_variable m_cv;
    bool m_stopping = false;
    Thread m_thread;
};




}
}
#endif
