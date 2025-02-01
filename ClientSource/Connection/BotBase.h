/*  Pokemon Automation Bot-Base Interface
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AbstractBotBase_H
#define PokemonAutomation_AbstractBotBase_H

#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/LifetimeSanitizer.h"

namespace PokemonAutomation{

class Logger;
struct BotBaseMessage;
class BotBaseRequest;
class BotBaseControllerContext;



class BotBaseController{
public:
    using ContextType = BotBaseControllerContext;

    enum class State{
        RUNNING,
        STOPPING,
        STOPPED,
//        ERROR,
    };

public:
    virtual ~BotBaseController() = default;

    virtual Logger& logger() = 0;
    virtual State state() const = 0;
    virtual size_t queue_limit() const = 0;

    //  Waits for all pending requests to finish.
    virtual void wait_for_all_requests(const Cancellable* cancelled = nullptr) = 0;

    //  Stop all pending commands. This wipes the command queue on both sides
    //  and stops any currently executing command.
    virtual bool try_stop_all_commands() = 0;
    virtual void stop_all_commands() = 0;

    //  Tell the device that the next command should replace the command queue.
    //
    //  So once the next command is issued, it will stop all ongoing commands
    //  and run the new command instead.
    //
    //  Once this function is called, all commands before it are no longer
    //  guaranteed to finish even if no command interrupts it.
    virtual bool try_next_command_interrupt() = 0;
    virtual void next_command_interrupt() = 0;

public:
    virtual bool try_issue_request(
        const BotBaseRequest& request,
        const Cancellable* cancelled = nullptr
    ) = 0;
    virtual void issue_request(
        const BotBaseRequest& request,
        const Cancellable* cancelled = nullptr
    ) = 0;
    virtual BotBaseMessage issue_request_and_wait(
        const BotBaseRequest& request,
        const Cancellable* cancelled = nullptr
    ) = 0;

};



class ControllerContext0 : public CancellableScope{
public:
    virtual void wait_for_all_requests() const = 0;
    virtual void cancel_now() = 0;
    virtual void cancel_lazy() = 0;
};


//  A wrapper for BotBase that allows for asynchronous cancelling.
class BotBaseControllerContext final : public ControllerContext0{
public:
    using ControllerType = BotBaseController;

public:
    BotBaseControllerContext(BotBaseController& botbase)
        : m_controller(botbase)
    {}
    BotBaseControllerContext(CancellableScope& parent, BotBaseController& botbase)
        : m_controller(botbase)
    {
        attach(parent);
    }
    virtual ~BotBaseControllerContext(){
        detach();
    }


    virtual void wait_for_all_requests() const override;

    operator BotBaseController&() const{ return m_controller; }
    BotBaseController& controller() const{ return m_controller; }

    //  Stop all commands in this context now.
    virtual void cancel_now() override;

    //  Stop the commands in this context, but do it lazily.
    //  Still will stop new commands from being issued to the device,
    //  and will tell the device that the next command that is issued
    //  should replace the command queue.
    //  This cancel is used when you need continuity from an ongoing
    //  sequence.
    virtual void cancel_lazy() override;


    virtual bool cancel(std::exception_ptr exception) noexcept override;


public:
    bool try_issue_request(const BotBaseRequest& request) const;
    void issue_request(const BotBaseRequest& request) const;
    BotBaseMessage issue_request_and_wait(const BotBaseRequest& request) const;


private:
    BotBaseController& m_controller;
    LifetimeSanitizer m_lifetime_sanitizer;
};





}


#endif

