/*  Async Command Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "InterruptableCommands.h"

namespace PokemonAutomation{



InterruptableCommandSession::CommandSet::CommandSet(
    BotBase& botbase,
    std::function<void(const BotBaseContext&)>&& lambda
)
    : context(botbase)
    , commands(std::move(lambda))
{}

InterruptableCommandSession::InterruptableCommandSession(BotBase& botbase, bool enable_interrupt)
    : m_enable_interrupt(enable_interrupt)
    , m_botbase(botbase)
{}

bool InterruptableCommandSession::run(std::function<void(const BotBaseContext&)>&& lambda){
    {
        SpinLockGuard lg(m_lock, "InterruptableCommandSession::run() - start");
        if (m_current){
            return false;
        }

        m_current.reset(new CommandSet(
            m_botbase, std::move(lambda)
        ));
    }
    while (true){
        if (m_current->context.botbase().state() != BotBase::State::RUNNING){
            throw CancelledException();
        }

        try{
            m_current->commands(m_current->context);
            break;
        }catch (CancelledException&){
            SpinLockGuard lg(m_lock, "InterruptableCommandSession::run() - cancelled");
            m_current = std::move(m_pending);
            if (m_current){
                continue;
            }else{
                break;
            }
        }
    }
    {
        SpinLockGuard lg(m_lock, "InterruptableCommandSession::run() - end");
        m_current.reset();
        m_pending.reset();
    }
    return true;
}


#if 0
void InterruptableCommandSession::interrupt_with(std::function<void(const BotBaseContext&)>&& lambda){
    SpinLockGuard lg(m_lock, "InterruptableCommandSession::interrupt_with()");
    if (m_current){
        m_current->context.cancel();
        m_pending.reset(new CommandSet(m_botbase, std::move(lambda)));
    }
}
#endif

void InterruptableCommandSession::stop(){
    if (!m_enable_interrupt.load(std::memory_order_acquire)){
        return;
    }
    SpinLockGuard lg(m_lock, "InterruptableCommandSession::stop()");
    if (m_current){
        m_current->context.cancel();
    }
}




}

