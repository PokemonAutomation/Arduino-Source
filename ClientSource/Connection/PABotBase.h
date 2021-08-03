/*  Pokemon Automation Bot Base
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      This is the main PABotBase class.
 * 
 *  This class represents a connection to a single PABotBase instance running on
 *  a user specified COM port. You can have multiple instances of this class if
 *  you are connecting to multiple devices at once.
 * 
 *  This class implements the full communication protocol. So you directly
 *  invoke commands from this class which will be passed on to the Arduino/Teensy.
 * 
 *  Requests and commands may be asynchronous. They may return before the device
 *  executes it.
 * 
 * 
 *      Note that button commands will only work if the device is running PABotBase
 *  and is not already running a command. The regular programs do not listen to
 *  button press requests since they are already running their own program.
 * 
 */

#ifndef PokemonAutomation_PABotBase_H
#define PokemonAutomation_PABotBase_H

#include <string.h>
#include <map>
#include <atomic>
#include <condition_variable>
#include <thread>
#include "Common/Cpp/SpinLock.h"
#include "ClientSource/Connection/PABotBaseConnection.h"
#include "ClientSource/Libraries/Logging.h"
#include "BotBase.h"

//#include <iostream>
//using std::cout;
//using std::endl;


namespace PokemonAutomation{


class PABotBase : public BotBase, private PABotBaseConnection{
    static const size_t MAX_PENDING_REQUESTS = PABB_DEVICE_QUEUE_SIZE;
    static const seqnum_t MAX_SEQNUM_GAP = (seqnum_t)-1 >> 2;

public:
    PABotBase(
        std::unique_ptr<StreamConnection> connection,
        MessageLogger* logger = nullptr,
        std::chrono::milliseconds retransmit_delay = std::chrono::milliseconds(PABB_RETRANSMIT_DELAY_MILLIS)
    );
    virtual ~PABotBase();

    using PABotBaseConnection::set_sniffer;

    void connect();
    void stop();

    std::chrono::time_point<std::chrono::system_clock> last_ack() const{
        return m_last_ack.load(std::memory_order_acquire);
    }

    virtual State state() const override{
        return m_state.load(std::memory_order_acquire);
    }

public:
    //  Basic Requests
    uint32_t    protocol_version    ();
    uint32_t    program_version     ();
    uint8_t     program_id          ();

    //  Waits for all pending requests to finish.
    virtual void wait_for_all_requests() override;

    //  Stop all pending commands. This wipes the command queue on both sides
    //  and stops any currently executing command.
    virtual void stop_all_commands() override;


public:
    //  For Command Implementations

    using BotBase::try_issue_request;
    using BotBase::issue_request;
    using BotBase::issue_request_and_wait;


private:
    enum class AckState{
        NOT_ACKED,
        ACKED,
        FINISHED,
    };
    struct PendingRequest{
        AckState state = AckState::NOT_ACKED;
        bool silent_remove;
        BotBaseMessage request;
        BotBaseMessage ack;
        std::chrono::system_clock::time_point first_sent;
    };
    struct PendingCommand{
        AckState state = AckState::NOT_ACKED;
        bool silent_remove;
        BotBaseMessage request;
        BotBaseMessage ack;
        std::chrono::system_clock::time_point first_sent;
    };

    template <typename Map>
    uint64_t infer_full_seqnum(const Map& map, seqnum_t seqnum) const;

    uint64_t oldest_live_seqnum() const;

    template <typename Params> void process_ack_request(BotBaseMessage message);
    template <typename Params> void process_ack_command(BotBaseMessage message);

    template <typename Params> void process_command_finished(BotBaseMessage message);
    virtual void on_recv_message(BotBaseMessage message) override;

    void remove_request(std::map<uint64_t, PendingRequest>::iterator iter);
    void remove_command(std::map<uint64_t, PendingCommand>::iterator iter);

    void retransmit_thread();

private:
    bool try_issue_request(
        std::map<uint64_t, PendingRequest>::iterator& iter,
        const std::atomic<bool>* cancelled,
        uint8_t send_type, void* send_params, size_t send_bytes,
        bool silent_remove, size_t queue_limit
    );
    bool try_issue_command(
        std::map<uint64_t, PendingCommand>::iterator& iter,
        const std::atomic<bool>* cancelled,
        uint8_t send_type, void* send_params, size_t send_bytes,
        bool silent_remove, size_t queue_limit
    );
    bool issue_request(
        std::map<uint64_t, PendingRequest>::iterator& iter,
        const std::atomic<bool>* cancelled,
        uint8_t send_type, void* send_params, size_t send_bytes,
        bool silent_remove
    );
    bool issue_command(
        std::map<uint64_t, PendingCommand>::iterator& iter,
        const std::atomic<bool>* cancelled,
        uint8_t send_type, void* send_params, size_t send_bytes,
        bool silent_remove
    );

    virtual bool try_issue_request(
        const std::atomic<bool>* cancelled,
        uint8_t send_type, void* send_params, size_t send_bytes
    ) override;
    virtual void issue_request(
        const std::atomic<bool>* cancelled,
        uint8_t send_type, void* send_params, size_t send_bytes
    ) override;
    virtual void issue_request_and_wait(
        const std::atomic<bool>* cancelled,
        uint8_t send_type, void* send_params, size_t send_bytes,
        uint8_t recv_type, void* recv_params, size_t recv_bytes
    ) override;


private:
    uint64_t m_send_seq;
    std::chrono::milliseconds m_retransmit_delay;
    std::atomic<std::chrono::time_point<std::chrono::system_clock>> m_last_ack;

    std::map<uint64_t, PendingRequest> m_pending_requests;
    std::map<uint64_t, PendingCommand> m_pending_commands;

    //  If you need both locks, always acquire m_sleep_lock first!
    SpinLock m_state_lock;
    std::mutex m_sleep_lock;

    std::condition_variable m_cv;
    std::atomic<State> m_state;
    std::thread m_retransmit_thread;
};







}

#endif
