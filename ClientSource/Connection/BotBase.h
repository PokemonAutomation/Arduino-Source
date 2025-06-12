/*  Pokemon Automation Bot-Base Interface
 * 
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AbstractBotBase_H
#define PokemonAutomation_AbstractBotBase_H

#include <string>
#include "Common/Cpp/CancellableScope.h"

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
    virtual void stop(std::string error_message = "") = 0;

    virtual Logger& logger() = 0;
    virtual State state() const = 0;
    virtual size_t queue_limit() const = 0;

    virtual void notify_all() = 0;

    //  Waits for all pending requests to finish.
    virtual void wait_for_all_requests(const Cancellable* cancelled = nullptr) = 0;

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





}


#endif

