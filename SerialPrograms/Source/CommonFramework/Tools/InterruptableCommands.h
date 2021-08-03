/*  Interruptable Commands
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_InterruptableCommands_H
#define PokemonAutomation_InterruptableCommands_H

#include <thread>
#include "Common/Cpp/SpinLock.h"
#include "Common/Cpp/AsyncDispatcher.h"
#include "ClientSource/Connection/BotBase.h"


namespace PokemonAutomation{


class InterruptableCommandSession{

public:
    InterruptableCommandSession(BotBase& botbase, bool enable_interrupt = true);
//    void set_interruptable(bool interruptable){
//        m_enable_interrupt.store(interruptable, std::memory_order_release);
//    }

public:
    //  Execution Thread

    //  Run commands. Return when all commands are finished.
    //  Returns false if there is already something running.
    bool run(std::function<void(const BotBaseContext&)>&& lambda);

public:
    //  External Threads

//    //  If there is a command running right now, interrupt it with a different one.
//    void interrupt_with(std::function<void(const BotBaseContext&)>&& lambda);

    //  Stop the currently running command.
    void stop();

private:
    struct CommandSet{
        CommandSet(BotBase& botbase, std::function<void(const BotBaseContext&)>&& lambda);
        BotBaseContext context;
        std::function<void(const BotBaseContext&)> commands;
    };

    SpinLock m_lock;
    std::atomic<bool> m_enable_interrupt;
    BotBase& m_botbase;
    std::unique_ptr<CommandSet> m_current;
    std::unique_ptr<CommandSet> m_pending;
};







}
#endif
