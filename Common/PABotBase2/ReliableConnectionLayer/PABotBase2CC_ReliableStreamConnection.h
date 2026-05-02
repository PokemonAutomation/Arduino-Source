/*  Reliable Stream Connection (CC)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2CC_ReliableStreamConnection_H
#define PokemonAutomation_PABotBase2CC_ReliableStreamConnection_H

#include "Common/Cpp/Time.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/ThreadPool.h"
#include "Common/Cpp/StreamConnections/PushingStreamConnections.h"
#include "PABotBase2_PacketSender.h"
#include "PABotBase2_PacketParser.h"
#include "PABotBase2_StreamCoalescer.h"

namespace PokemonAutomation{
namespace PABotBase2{



class ReliableStreamConnection final
    : public CancellableScope
    , public ReliableStreamConnectionPushing
    , private UnreliableStreamSender
    , private PacketRunner
    , private StreamListener
{
public:
    ReliableStreamConnection(
        CancellableScope* parent,
        Logger& logger, bool log_everything,
        ThreadPool& thread_pool,
        UnreliableStreamConnectionPushing& unreliable_connection,
        WallDuration retransmit_timeout = Milliseconds(100),
        Mutex* print_lock = nullptr
    );
    ~ReliableStreamConnection();

    virtual void stop() override{
        cancel(nullptr);
    }
    virtual bool cancel(std::exception_ptr exception) noexcept override;

    bool reset(WallDuration timeout = WallDuration::max());

    bool remote_protocol_is_compatible() const{
        return m_remote_protocol_compatible;
    }
    uint32_t remote_protocol() const{
        return m_remote_protocol;
    }
    const std::string& error_string() const{
        return m_error;
    }

    size_t pending() const;
    bool wait_for_pending(WallDuration timeout = WallDuration::max());


public:
    //  Send in-band

    bool try_send_request(uint8_t opcode);
    void send_request(uint8_t opcode);

    void send_stream(const void* data, size_t bytes){
        reliable_send_all_or_nothing(data, bytes, WallDuration::max());
    }


public:
    //  Debugging

    void print() const;
    Mutex* print_lock() const{
        return m_print_lock;
    }



private:
    //  Send

    void send_ack(uint8_t seqnum, uint8_t opcode);
    void send_ack_u16(uint8_t seqnum, uint8_t opcode, uint16_t data);

    void retransmit_thread();


private:
    virtual bool reliable_send_all_or_nothing(
        const void* data, size_t bytes,
        WallDuration timeout
    ) noexcept override;
    virtual void on_recv(const void* data, size_t bytes) override;
    virtual size_t unreliable_send(const void* data, size_t bytes) noexcept override;


private:
    //  Virtuals: PacketRunner

    virtual void on_packet(const PacketHeader* packet) override;

    void process_UNKNOWN_OPCODE(const PacketHeader* packet);
    void process_RET_RESET(const PacketHeader* packet);
    void process_RET_VERSION(const PacketHeader* packet);
    void process_RET_PACKET_SIZE(const PacketHeader* packet);
    void process_RET_BUFFER_SLOTS(const PacketHeader* packet);
    void process_RET_BUFFER_BYTES(const PacketHeader* packet);

    void process_ASK_STREAM_DATA(const PacketHeader* packet);
    void process_RET_STREAM_DATA(const PacketHeader* packet);


private:
    Logger& m_logger;
    UnreliableStreamConnectionPushing& m_unreliable_connection;
    const WallDuration m_retransmit_timeout;
    Mutex* m_print_lock;

    PacketSender m_reliable_sender;
    PacketParser m_parser;
    StreamCoalescer m_stream_coalescer;

    bool m_log_everything;

    bool m_remote_protocol_compatible;
    uint32_t m_remote_protocol;

//    std::atomic<bool> m_version_verified;
    uint8_t m_remote_slot_capacity;
    uint16_t m_remote_buffer_capacity;

    std::string m_error;

    mutable Mutex m_lock;
    ConditionVariable m_cv;

    AsyncTask m_retransmit_thread;
};



}
}
#endif
