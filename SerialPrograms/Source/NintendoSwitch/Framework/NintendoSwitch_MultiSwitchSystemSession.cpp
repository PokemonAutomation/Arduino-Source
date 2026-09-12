/*  Multi-Switch System Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/EarlyShutdown.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/Logging/Logger.h"
#include "NintendoSwitch_MultiSwitchSystemSession.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



void MultiSwitchSystemSession::add_listener(Listener& listener){
    std::lock_guard<Mutex> lg(m_lock);
    m_listeners.insert(&listener);
    listener.startup(m_consoles.size());
}
void MultiSwitchSystemSession::remove_listener(Listener& listener){
    std::lock_guard<Mutex> lg(m_lock);
    m_listeners.erase(&listener);
}


bool MultiSwitchSystemSession::try_shutdown(){
    bool success = true;
    for (SwitchSystemSession& console : m_consoles){
        success &= console.try_shutdown();
    }
    return success;
}
MultiSwitchSystemSession::~MultiSwitchSystemSession(){
    blocking_shutdown(
        global_logger_tagged(),
        "MultiSwitchSystemSession",
        [this]{ return try_shutdown(); }
    );
}

MultiSwitchSystemSession::MultiSwitchSystemSession(
    MultiSwitchSystemOption& option,
    bool allow_commands_while_locked,
    uint64_t program_id
)
    : m_option(option)
    , m_allow_commands_while_locked(allow_commands_while_locked)
    , m_program_id(program_id)
    , m_switch_count_locked(false)
    , m_consoles(option.active_consoles())
{
    size_t count = option.active_consoles();
    for (size_t c = 0; c < count; c++){
        m_consoles.emplace_back(option[c], allow_commands_while_locked, c, program_id);
    }
}

void MultiSwitchSystemSession::lock(){
    std::lock_guard<Mutex> lg(m_lock);
    m_switch_count_locked = true;
}
void MultiSwitchSystemSession::unlock(){
    std::lock_guard<Mutex> lg(m_lock);
    m_switch_count_locked = false;
}
bool MultiSwitchSystemSession::set_switch_count(size_t count){
    std::lock_guard<Mutex> lg(m_lock);
    if (m_switch_count_locked){
        return false;
    }

    for (Listener* listener : m_listeners){
        listener->shutdown();
    }
    m_consoles.reset(count);
    m_option.resize(count);
    for (size_t c = 0; c < count; c++){
        m_consoles.emplace_back(m_option[c], m_allow_commands_while_locked, c, m_program_id);
    }
    for (Listener* listener : m_listeners){
        listener->startup(count);
    }

    return true;
}



JsonValue MultiSwitchSystemSession::to_json() const{
    return m_option.to_json();
}
void MultiSwitchSystemSession::load_json(const JsonValue& json){
    m_option.load_json(json);
    set_switch_count(m_option.active_consoles());
}





}
}
