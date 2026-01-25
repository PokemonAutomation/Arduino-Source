/*  Button Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <set>
#include <atomic>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Strings/StringTools.h"
//#include "Common/Cpp/Json/JsonValue.h"
#include "ButtonOption.h"

namespace PokemonAutomation{


struct ButtonCell::Data{
    std::atomic<Enabled> m_state;

    mutable SpinLock m_text_lock;
    std::string m_text;

    std::atomic<int> m_button_height;
    std::atomic<int> m_text_size;

    mutable SpinLock m_listener_lock;
    std::set<ButtonListener*> m_listeners;

    Data(
        Enabled state,
        std::string text,
        int button_height,
        int text_size
    )
        : m_state(state)
        , m_text(std::move(text))
        , m_button_height(button_height)
        , m_text_size(text_size)
    {}

    std::string text() const{
        ReadSpinLock lg(m_text_lock);
        return m_text;
    }
};

ButtonCell::~ButtonCell() = default;
ButtonCell::ButtonCell(const ButtonCell& x)
    : ConfigOptionImpl<ButtonCell>(x)
    , m_data(
        CONSTRUCT_TOKEN,
        x.m_data->m_state,
        x.m_data->text(),
        x.m_data->m_button_height.load(std::memory_order_relaxed),
        x.m_data->m_text_size.load(std::memory_order_relaxed)
    )
{}
ButtonCell::ButtonCell(
    std::string text,
    int button_height,
    int text_size
)
    : ConfigOptionImpl<ButtonCell>(LockMode::UNLOCK_WHILE_RUNNING)
    , m_data(CONSTRUCT_TOKEN, ButtonCell::ENABLED, std::move(text), button_height, text_size)
{}
ButtonCell::ButtonCell(
    Enabled state,
    std::string text,
    int button_height,
    int text_size
)
    : ConfigOptionImpl<ButtonCell>(LockMode::UNLOCK_WHILE_RUNNING)
    , m_data(CONSTRUCT_TOKEN, state, std::move(text), button_height, text_size)
{}

void ButtonCell::add_listener(ButtonListener& listener){
    WriteSpinLock lg(m_data->m_listener_lock);
    m_data->m_listeners.insert(&listener);
}
void ButtonCell::remove_listener(ButtonListener& listener){
    WriteSpinLock lg(m_data->m_listener_lock);
    m_data->m_listeners.erase(&listener);
}

bool ButtonCell::is_enabled() const{
    return m_data->m_state.load(std::memory_order_relaxed) == Enabled::ENABLED;
}
void ButtonCell::set_enabled(bool enabled){
    Enabled e = enabled ? ENABLED : DISABLED;
    if (e != m_data->m_state.exchange(e, std::memory_order_relaxed)){
        report_value_changed(this);
    }
}
std::string ButtonCell::text() const{
    return m_data->text();
}
int ButtonCell::button_height() const{
    return m_data->m_button_height.load(std::memory_order_relaxed);
}
int ButtonCell::text_size() const{
    return m_data->m_text_size.load(std::memory_order_relaxed);
}
void ButtonCell::set_text(std::string text){
    {
        WriteSpinLock lg(m_data->m_text_lock);
        if (text == m_data->m_text){
            return;
        }
        m_data->m_text = std::move(text);
    }
    report_value_changed(this);
}

void ButtonCell::press_button(){
    ReadSpinLock lg(m_data->m_listener_lock);
    for (ButtonListener* listener : m_data->m_listeners){
        listener->on_press();
    }
}

//void ButtonCell::restore_defaults(){
//    *this = m_data->m_default;
//}






struct ButtonOption::Data{
    mutable SpinLock m_label_lock;
    std::string m_label;

    Data(std::string label)
        : m_label(std::move(label))
    {}
};


ButtonOption::~ButtonOption() = default;
ButtonOption::ButtonOption(
    std::string label,
    std::string text,
    int button_height,
    int text_size
)
    : ConfigOptionImpl<ButtonOption, ButtonCell>(std::move(text), button_height, text_size)
    , m_data(CONSTRUCT_TOKEN, std::move(label))
{}
ButtonOption::ButtonOption(
    std::string label,
    Enabled state,
    std::string text,
    int button_height,
    int text_size
)
    : ConfigOptionImpl<ButtonOption, ButtonCell>(state, std::move(text), button_height, text_size)
    , m_data(CONSTRUCT_TOKEN, std::move(label))
{}


std::string ButtonOption::label() const{
    ReadSpinLock lg(m_data->m_label_lock);
    return m_data->m_label;
}
void ButtonOption::set_label(std::string label){
    {
        WriteSpinLock lg(m_data->m_label_lock);
        if (label == m_data->m_label){
            return;
        }
        m_data->m_label = std::move(label);
    }
    report_value_changed(this);
}





struct DeferredStopButtonOption::Data{
    std::atomic<bool> m_stop_requested;
    std::string m_ready_text;
    std::string m_pressed_text;

    Data(const std::string& iteration_name)
        : m_stop_requested(false)
        , m_ready_text("Stop after Current " + StringTools::capitalize(iteration_name))
        , m_pressed_text("Program will stop after current " + StringTools::uncapitalize(iteration_name) + "...")
    {}
};


DeferredStopButtonOption::ResetOnExit::ResetOnExit(DeferredStopButtonOption& button)
    : m_button(button)
{
    button.reset_and_ready();
}

DeferredStopButtonOption::ResetOnExit::~ResetOnExit(){
    m_button.set_idle();
}


DeferredStopButtonOption::~DeferredStopButtonOption(){
}

DeferredStopButtonOption::DeferredStopButtonOption(
    const std::string& iteration_name,
    int button_height,
    int text_size
)
    : ButtonOption(
        "<b>Stop after current " + StringTools::uncapitalize(iteration_name) + ":",
        ButtonCell::DISABLED,
        "Stop after Current " + StringTools::capitalize(iteration_name),
        button_height,
        text_size
    )
    , m_data(CONSTRUCT_TOKEN, iteration_name)
{}

bool DeferredStopButtonOption::should_stop() const{
    return m_data->m_stop_requested.load(std::memory_order_relaxed);
}

void DeferredStopButtonOption::reset_and_ready(){
    m_data->m_stop_requested.store(false, std::memory_order_relaxed);
    this->set_enabled(true);
    this->set_text(m_data->m_ready_text);
}

void DeferredStopButtonOption::set_idle(){
    this->set_enabled(false);
    this->set_text(m_data->m_ready_text);
}

void DeferredStopButtonOption::press_button(){
    m_data->m_stop_requested.store(true, std::memory_order_relaxed);
    this->set_enabled(false);
    this->set_text(m_data->m_pressed_text);
}


}
