/*  PABotBase2 Packet Sender
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PABotBase2_CC_PacketSender.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace PABotBase2{


PacketSender::PacketSender(
    Logger& logger,
    StreamSender& sender,
    WallDuration retransmit_timeout
)
    : m_logger(logger)
    , m_sender(sender)
    , m_retransmit_timeout(retransmit_timeout)
{
    pabb2_PacketSender_init(&m_queue, sender_fp, &m_sender);
    m_thread = Thread([this]{ retransmit_thread(); });
}
PacketSender::~PacketSender(){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        m_stopping = true;
    }
    m_cv.notify_all();
    m_thread.join();
}

bool PacketSender::send_packet(uint8_t opcode, uint8_t extra_bytes, const void* extra_data){
    return pabb2_PacketSender_send_packet(&m_queue, opcode, extra_bytes, extra_data);
}
size_t PacketSender::send_stream(const uint8_t* data, size_t bytes){
    return pabb2_PacketSender_send_stream(&m_queue, data, bytes);
}
void PacketSender::retransmit_thread(){
    WallClock next_retransmit = current_time() + m_retransmit_timeout;
    while (true){
        std::unique_lock<std::mutex> lg(m_lock);

        m_cv.wait_until(lg, next_retransmit);

        if (m_stopping){
            break;
        }
        if (pabb2_PacketSender_size(&m_queue) == 0){
            continue;
        }
        if (current_time() < next_retransmit){
            continue;
        }

        cout << "running retransmits" << endl;

        if (!pabb2_PacketSender_iterate_retransmits(&m_queue)){
            cout << "nothing to do" << endl;
            next_retransmit = current_time() + m_retransmit_timeout;
        }else{
            cout << "did something" << endl;
        }
    }
}



}
}
