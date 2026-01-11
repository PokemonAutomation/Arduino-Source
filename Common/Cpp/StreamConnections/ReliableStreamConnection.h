/*  Reliable Stream Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ReliableStreamConnection_H
#define PokemonAutomation_ReliableStreamConnection_H

#include <mutex>
#include <condition_variable>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Concurrency/Thread.h"
#include "Common/PABotBase2/PABotBase2_PacketSender.h"
#include "Common/PABotBase2/PABotBase2_PacketParser.h"
#include "Common/PABotBase2/PABotBase2_StreamCoalescer.h"
#include "StreamConnection.h"

namespace PokemonAutomation{



class ReliableStreamConnection final : public StreamConnection, private StreamListener{
public:
    ReliableStreamConnection(
        Logger& logger,
        StreamConnection& unreliable_connection,
        WallDuration retransmit_timeout = Milliseconds(100)
    );
    ~ReliableStreamConnection();

    virtual void stop() override;

    virtual size_t send(const void* data, size_t bytes) override;

    bool send_request(uint8_t opcode);


private:
    //  Send

    void send_ack(uint8_t seqnum);

    static void send_raw(void* context, const void* data, size_t bytes){
        ReliableStreamConnection& self = *(ReliableStreamConnection*)context;
        self.m_unreliable_connection.send(data, bytes);
    }
    void retransmit_thread();


private:
    //  Receive

    virtual void on_recv(const void* data, size_t bytes) override{
        pabb2_PacketParser_push_bytes(&m_parser, (const uint8_t*)data, bytes);
    }
    static void on_packet(void* context, const pabb2_PacketHeader* packet){
        ReliableStreamConnection& self = *(ReliableStreamConnection*)context;
        self.on_packet(packet);
    }
    void on_packet(const pabb2_PacketHeader* packet);



private:
    Logger& m_logger;
    StreamConnection& m_unreliable_connection;
    const WallDuration m_retransmit_timeout;

    pabb2_PacketSender m_reliable_sender;
    pabb2_PacketParser m_parser;
    pabb2_StreamCoalescer m_stream_coalescer;

    mutable std::mutex m_lock;
    std::condition_variable m_cv;
    bool m_stopping = false;
    Thread m_retransmit_thread;
};




}
#endif
