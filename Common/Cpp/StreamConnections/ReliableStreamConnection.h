/*  Reliable Stream Connection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ReliableStreamConnection_H
#define PokemonAutomation_ReliableStreamConnection_H

#include "Common/Cpp/Time.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "Common/PABotBase2/PABotBase2_PacketSender.h"
#include "Common/PABotBase2/PABotBase2_PacketParser.h"
#include "Common/PABotBase2/PABotBase2_StreamCoalescer.h"
#include "StreamConnection.h"

namespace PokemonAutomation{



class ReliableStreamConnection final
    : public CancellableScope
    , public StreamConnection
    , private StreamListener
{
public:
    ReliableStreamConnection(
        CancellableScope* parent,
        Logger& logger, bool log_everything,
        ThreadPool& thread_pool,
        StreamConnection& unreliable_connection,
        WallDuration retransmit_timeout = Milliseconds(100)
    );
    ~ReliableStreamConnection();

    virtual void stop() override{
        cancel(nullptr);
    }
    virtual bool cancel(std::exception_ptr exception) noexcept override;
    void wait_for_pending();

    void reset();

    //  Send stream data.
    virtual size_t send(const void* data, size_t bytes) override;

    bool try_send_request(uint8_t opcode);
    void send_request(uint8_t opcode);


public:
    void print() const;



private:
    //  Send

    void send_ack(uint8_t seqnum, uint8_t opcode);
    void send_ack_u16(uint8_t seqnum, uint8_t opcode, uint16_t data);

    static size_t send_raw(
        void* context,
        const void* data, size_t bytes,
        bool is_retransmit
    );
    void retransmit_thread();


private:
    //  Receive

    virtual void on_recv(const void* data, size_t bytes) override;
    static void on_packet(void* context, const pabb2_PacketHeader* packet){
        ReliableStreamConnection& self = *(ReliableStreamConnection*)context;
        self.on_packet(packet);
    }
    void on_packet(const pabb2_PacketHeader* packet);

    void process_RET_RESET(const pabb2_PacketHeader* packet);
    void process_RET_VERSION(const pabb2_PacketHeader* packet);
    void process_RET_PACKET_SIZE(const pabb2_PacketHeader* packet);
    void process_RET_BUFFER_SLOTS(const pabb2_PacketHeader* packet);
    void process_RET_BUFFER_BYTES(const pabb2_PacketHeader* packet);

    void process_ASK_STREAM_DATA(const pabb2_PacketHeader* packet);
    void process_RET_STREAM_DATA(const pabb2_PacketHeader* packet);


private:
//public:

    Logger& m_logger;
    StreamConnection& m_unreliable_connection;
    const WallDuration m_retransmit_timeout;

    pabb2_PacketSender m_reliable_sender;
    pabb2_PacketParser m_parser;
    pabb2_StreamCoalescer m_stream_coalescer;

    bool m_log_everything;
//    std::atomic<bool> m_version_verified;
    uint8_t m_remote_slot_capacity;
    uint16_t m_remote_buffer_capacity;

    std::string m_error;

    mutable Mutex m_lock;
    ConditionVariable m_cv;
    AsyncTask m_retransmit_thread;
};




}
#endif
