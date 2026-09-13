/*  Multi-Console System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Logging/GlobalLogger.h"
#include "Common/Cpp/EarlyShutdown.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/Logging/Logger.h"
#include "MultiConsoleSystemSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace GameConsole{


void MultiConsoleSystemSession::add_listener(Listener& listener){
    m_listeners.add(listener);
}
void MultiConsoleSystemSession::remove_listener(Listener& listener){
    m_listeners.remove(listener);
}

bool MultiConsoleSystemSession::try_shutdown(){
    bool success = true;
    for (std::unique_ptr<ConsoleSystemSession>& console : m_consoles){
        success &= console->try_shutdown();
    }
    return success;
}
MultiConsoleSystemSession::~MultiConsoleSystemSession(){
    blocking_shutdown(
        global_logger_tagged(),
        "MultiConsoleSystemSession",
        [this]{ return try_shutdown(); }
    );
}

MultiConsoleSystemSession::MultiConsoleSystemSession(
    MultiConsoleSystemOption& option,
    bool allow_commands_while_locked,
    std::optional<uint64_t> program_tracking_id
)
    : MultiConsoleSystemSession(
        option,
        [allow_commands_while_locked, program_tracking_id](ConsoleSystemOption& option, size_t console_index){
            return std::make_unique<ConsoleSystemSession>(
                global_logger_raw(),
                option,
                allow_commands_while_locked,
                console_index,
                program_tracking_id
            );
        }
    )
{}
MultiConsoleSystemSession::MultiConsoleSystemSession(
    MultiConsoleSystemOption& option,
    std::function<
        std::unique_ptr<ConsoleSystemSession>(
            ConsoleSystemOption& option,
            size_t console_index
        )
    > factory
)
    : m_option(option)
    , m_consoles(option.active_consoles())
    , m_factory(std::move(factory))
{
    size_t count = option.active_consoles();
    for (size_t c = 0; c < count; c++){
        m_consoles.emplace_back(m_factory(option[c], c));
    }
}

void MultiConsoleSystemSession::lock_controllers(const std::string& reason){
    std::unique_lock<Mutex> lg1(m_state_lock);
    m_listeners.run_method(&Listener::on_console_count_lock, true);
    for (std::unique_ptr<ConsoleSystemSession>& console : m_consoles){
        console->lock_controllers(reason);
    }
}
void MultiConsoleSystemSession::unlock_controllers(){
    std::unique_lock<Mutex> lg1(m_state_lock);
    for (std::unique_ptr<ConsoleSystemSession>& console : m_consoles){
        console->unlock_controllers();
    }
    m_listeners.run_method(&Listener::on_console_count_lock, false);
}


void MultiConsoleSystemSession::add_console_count_lock(Listener& listener){
//    cout << "add_console_count_lock()" << endl;
    std::lock_guard<Mutex> lg(m_state_lock);
    m_count_locks.insert(&listener);
}
void MultiConsoleSystemSession::remove_console_count_lock(Listener& listener){
//    cout << "remove_console_count_lock()" << endl;
    std::lock_guard<Mutex> lg(m_state_lock);
    m_count_locks.erase(&listener);
}

void MultiConsoleSystemSession::set_active_consoles(size_t count){
    {
        std::lock_guard<Mutex> lg(m_state_lock);
        if (count == m_option.active_consoles() &&
            count == m_consoles.size()
        ){
            return;
        }
    }

    std::lock_guard<Mutex> lg0(m_resize_lock);

    //  Signal all listeners to drop locks.
    m_listeners.run_method(&Listener::shutdown);

    {
        std::unique_lock<Mutex> lg1(m_state_lock);

        //  Wait for all locks to be dropped.
        m_cv.wait(lg1, [this]{
            return m_count_locks.empty();
        });

        m_consoles.reset(count);
        m_option.resize(count);
        for (size_t c = 0; c < count; c++){
            m_consoles.emplace_back(m_factory(m_option[c], c));
        }
    }

    //  Signal all listeners it's ok to re-lock.
    m_listeners.run_method(&Listener::startup, count);
}


JsonValue MultiConsoleSystemSession::to_json() const{
    std::lock_guard<Mutex> lg(m_state_lock);
    return m_option.to_json();
}
void MultiConsoleSystemSession::load_json(const JsonValue& json){
//    cout << "MultiConsoleSystemSession::load_json()" << endl;
    {
        std::lock_guard<Mutex> lg(m_state_lock);
        m_option.load_json(json);
    }
    set_active_consoles(m_option.active_consoles());
}




}
}
