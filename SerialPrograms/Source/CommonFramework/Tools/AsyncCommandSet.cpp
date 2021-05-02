/*  Async Command Set
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "AsyncCommandSet.h"

namespace PokemonAutomation{


AsyncCommandSet::AsyncCommandSet(BotBase& botbase)
    : m_context(botbase)
    , m_thread(&AsyncCommandSet::thread_body, this)
{}
AsyncCommandSet::AsyncCommandSet(BotBase& botbase, AsyncDispatcher& dispatcher)
    : m_context(botbase)
    , m_task(dispatcher.dispatch([this]{ thread_body(); }))
{}
AsyncCommandSet::~AsyncCommandSet(){
    if (m_thread.joinable()){
        m_thread.join();
    }
}

void AsyncCommandSet::cancel(){
    if (m_thread.joinable()){
        m_context.cancel();
        m_thread.join();
    }
    if (m_task){
        m_context.cancel();
        m_task->wait();
    }
}

void AsyncCommandSet::thread_body(){
    try{
        task();
    }catch (CancelledException&){}
}



}

