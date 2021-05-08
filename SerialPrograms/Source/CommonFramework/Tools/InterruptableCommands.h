/*  Interruptable Commands
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_InterruptableCommands_H
#define PokemonAutomation_InterruptableCommands_H

#include <thread>
#include "Common/Clientside/SpinLock.h"
#include "Common/Clientside/AsyncDispatcher.h"
#include "ClientSource/Connection/BotBase.h"


namespace PokemonAutomation{


class InterruptableCommandSession{

public:
    InterruptableCommandSession(BotBase& botbase);

public:
    //  Execution Thread

    //  Run commands. Return when all commands are finished.
    //  Returns false if there is already something running.
    bool run(std::function<void(const BotBaseContext&)>&& lambda);

public:
    //  External Threads

    //  If there is a command running right now, interrupt it with a different one.
    void interrupt_with(std::function<void(const BotBaseContext&)>&& lambda);

    //  Stop the currently running command.
    void stop();

private:
    struct CommandSet{
        CommandSet(BotBase& botbase, std::function<void(const BotBaseContext&)>&& lambda);
        BotBaseContext context;
        std::function<void(const BotBaseContext&)> commands;
    };

    SpinLock m_lock;
    BotBase& m_botbase;
    std::unique_ptr<CommandSet> m_current;
    std::unique_ptr<CommandSet> m_pending;
};







}
#endif
