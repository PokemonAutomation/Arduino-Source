/*  Pokemon Automation Bot-Base Interface
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AbstractBotBase_H
#define PokemonAutomation_AbstractBotBase_H

#include <atomic>
#include "Common/Cpp/AbstractLogger.h"

namespace PokemonAutomation{


struct BotBaseMessage;
class BotBaseRequest;



class BotBase{
public:
    enum class State{
        RUNNING,
        STOPPING,
        STOPPED,
    };

public:
    virtual ~BotBase() = default;

    virtual Logger& logger() = 0;
    virtual State state() const = 0;

    //  Waits for all pending requests to finish.
    virtual void wait_for_all_requests(const std::atomic<bool>* cancelled = nullptr) = 0;

    //  Stop all pending commands. This wipes the command queue on both sides
    //  and stops any currently executing command.
    virtual void stop_all_commands() = 0;

    //  Tell the device that the next command should replace the command queue.
    //
    //  So once the next command is issued, it will stop all ongoing commands
    //  and run the new command instead.
    //
    //  Once this function is called, all commands before it are no longer
    //  guaranteed to finish even if no command interrupts it.
    virtual void next_command_interrupt() = 0;

public:
    virtual bool try_issue_request(
        const BotBaseRequest& request,
        const std::atomic<bool>* cancelled = nullptr
    ) = 0;
    virtual void issue_request(
        const BotBaseRequest& request,
        const std::atomic<bool>* cancelled = nullptr
    ) = 0;
    virtual BotBaseMessage issue_request_and_wait(
        const BotBaseRequest& request,
        const std::atomic<bool>* cancelled = nullptr
    ) = 0;

};



//  A wrapper for BotBase that allows for asynchronous cancelling.
class BotBaseContext{
public:
    BotBaseContext(BotBase& botbase)
        : m_botbase(botbase)
        , m_cancelled(false)
    {}

    void wait_for_all_requests() const{
        m_botbase.wait_for_all_requests(&m_cancelled);
    }

    //  Don't use this unless you really need to.
    BotBase& botbase() const{ return m_botbase; }

    //  Stop all commands in this context now.
    void cancel_now(){
        m_cancelled.store(true, std::memory_order_release);
        m_botbase.stop_all_commands();
    }

    //  Stop the commands in this context, but do it lazily.
    //  Still will stop new commands from being issued to the device,
    //  and will tell the device that the next command that is issued
    //  should replace the command queue.
    //  This cancel is used when you need continuity from an ongoing
    //  sequence.
    void cancel_lazy(){
        m_cancelled.store(true, std::memory_order_release);
        m_botbase.next_command_interrupt();
    }

    const std::atomic<bool>& cancelled_bool() const{
        return m_cancelled;
    }


public:
    bool try_issue_request(const BotBaseRequest& request) const{
        return m_botbase.try_issue_request(request, &m_cancelled);
    }
    void issue_request(const BotBaseRequest& request) const{
        m_botbase.issue_request(request, &m_cancelled);
    }
    BotBaseMessage issue_request_and_wait(const BotBaseRequest& request) const;


private:
    BotBase& m_botbase;
    std::atomic<bool> m_cancelled;
};





}


#endif

