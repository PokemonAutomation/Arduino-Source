/*  Keyboard Event Handler
 *
 */
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "KeyboardEventHandler.h"

namespace PokemonAutomation{



struct KeyboardEventHandler::Data{
    ListenerSet<KeyboardListener> m_listeners;
};

KeyboardEventHandler::KeyboardEventHandler()
    : m_data(CONSTRUCT_TOKEN)
{}

KeyboardEventHandler::~KeyboardEventHandler() = default;

void KeyboardEventHandler::add_listener(KeyboardListener& listener){
    auto scope = m_lifetime_sanitizer.check_scope();
    Data& data = *m_data;
    data.m_listeners.add(listener);
}
void KeyboardEventHandler::remove_listener(KeyboardListener& listener){
    auto scope = m_lifetime_sanitizer.check_scope();
    Data& data = *m_data;
    data.m_listeners.remove(listener);
}


}