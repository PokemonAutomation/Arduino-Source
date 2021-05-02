/*  Async Command Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AsyncCommandSet_H
#define PokemonAutomation_AsyncCommandSet_H

#include <thread>
#include "Common/Clientside/AsyncDispatcher.h"
#include "ClientSource/Connection/BotBase.h"


namespace PokemonAutomation{


class AsyncCommandSet{
public:
    AsyncCommandSet(BotBase& botbase);
    AsyncCommandSet(BotBase& botbase, AsyncDispatcher& dispatcher);
    ~AsyncCommandSet();

    void cancel();

    virtual void task() = 0;

private:
    void thread_body();

protected:
    BotBaseContext m_context;
private:
    std::thread m_thread;
    std::unique_ptr<AsyncTask> m_task;
};





}
#endif
