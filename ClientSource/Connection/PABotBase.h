/*  Pokemon Automation Bot Base
 * 
 *  From: https://github.com/PokemonAutomation/
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
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/SerialPABotBase/SerialPABotBase_Protocol.h"
#include "ClientSource/Connection/MessageLogger.h"
#include "ClientSource/Connection/PABotBaseConnection.h"
#include "BotBase.h"
#include "BotBaseMessage.h"


namespace PokemonAutomation{


class PABotBase : public BotBaseController, private PABotBaseConnection{
    static const seqnum_t MAX_SEQNUM_GAP = (seqnum_t)-1 >> 2;

public:
    PABotBase(
        Logger& logger,
        std::unique_ptr<StreamConnection> connection,
        MessageLogger* message_logger = nullptr,
        std::chrono::milliseconds retransmit_delay = std::chrono::milliseconds(100)
    );
    virtual ~PABotBase();

    using PABotBaseConnection::set_sniffer;

    void connect();
    virtual void stop(std::string error_message = "") override;

    std::chrono::time_point<std::chrono::system_clock> last_ack() const{
        return m_last_ack.load(std::memory_order_acquire);
    }

    virtual Logger& logger() override{
        return m_logger;
    }
    virtual State state() const override{
        return m_state.load(std::memory_order_acquire);
    }
    virtual void notify_all() override;

    virtual size_t queue_limit() const override{
        return m_max_pending_requests.load(std::memory_order_relaxed);
    }
    void set_queue_limit(size_t queue_limit);

public:
    //  Basic Requests

    virtual void wait_for_all_requests(const Cancellable* cancelled = nullptr) override;
    virtual void stop_all_commands() override;
    virtual void next_command_interrupt() override;


public:
    //  For Command Implementations

//    using BotBaseController::try_issue_request;
    using BotBaseController::issue_request;
    using BotBaseController::issue_request_and_wait;


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
        WallClock first_sent;
        LifetimeSanitizer sanitizer;
    };
    struct PendingCommand{
        AckState state = AckState::NOT_ACKED;
        bool silent_remove;
        BotBaseMessage request;
        BotBaseMessage ack;
        WallClock first_sent;
        LifetimeSanitizer sanitizer;
    };

    template <typename Map>
    uint64_t infer_full_seqnum(const Map& map, seqnum_t seqnum) const;

    uint64_t oldest_live_seqnum() const;

    template <typename Params, bool variable_length = false>
    void process_ack_request(BotBaseMessage message);
    template <typename Params>
    void process_ack_command(BotBaseMessage message);

    template <typename Params> void process_command_finished(BotBaseMessage message);
    virtual void on_recv_message(BotBaseMessage message) override;

    void clear_all_active_commands(uint64_t seqnum);

    void retransmit_thread();

private:
    size_t inflight_requests();

    //  Returns the seqnum of the request. If failed, returns zero.
    uint64_t try_issue_request(
        const Cancellable* cancelled,
        const BotBaseRequest& request,
        bool silent_remove, bool do_not_block
    );
    uint64_t try_issue_command(
        const Cancellable* cancelled,
        const BotBaseRequest& request,
        bool silent_remove
    );

    //  Returns the seqnum of the request.
    uint64_t issue_request(
        const Cancellable* cancelled,
        const BotBaseRequest& request,
        bool silent_remove, bool do_not_block
    );
    uint64_t issue_command(
        const Cancellable* cancelled,
        const BotBaseRequest& request,
        bool silent_remove
    );

public:
    virtual bool try_issue_request(
        const BotBaseRequest& request,
        const Cancellable* cancelled
    ) override;
    virtual void issue_request(
        const BotBaseRequest& request,
        const Cancellable* cancelled
    ) override;
    virtual BotBaseMessage issue_request_and_wait(
        const BotBaseRequest& request,
        const Cancellable* cancelled
    ) override;

private:
    BotBaseMessage wait_for_request(uint64_t seqnum, const Cancellable* cancelled = nullptr);

private:
    Logger& m_logger;

    std::atomic<size_t> m_max_pending_requests;

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
    std::atomic<bool> m_error;
    std::string m_error_message;
    std::thread m_retransmit_thread;

    LifetimeSanitizer m_sanitizer;
};







}

#endif
