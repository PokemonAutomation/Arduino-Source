/*  Sleep Suppress Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "SleepSuppressOption.h"

namespace PokemonAutomation{


SleepSuppressOption::SleepSuppressOption(std::string label, SleepSuppress default_value)
    : EnumDropdownOption<SleepSuppress>(
        std::move(label),
        {
           {SleepSuppress::NONE,       "none",         "Do not suppress sleep or screensaver."},
#ifdef PA_ENABLE_SLEEP_SUPPRESS_NO_SLEEP
           {SleepSuppress::NO_SLEEP,   "no-sleep",     "Prevent computer from sleeping, but allow screen to turn off."},
#endif
           {SleepSuppress::SCREEN_ON,  "screen-on",    "Keep the screen on."},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        default_value
    )
{}


SleepSuppressOptions::~SleepSuppressOptions(){
    PROGRAM_RUNNING.remove_listener(*this);
    IDLE.remove_listener(*this);
}
SleepSuppressOptions::SleepSuppressOptions()
    : GroupOption(
        "Suppress Screensaver/Sleep:",
        LockMode::UNLOCK_WHILE_RUNNING,
        GroupOption::EnableMode::ALWAYS_ENABLED, true
    )
    , IDLE("No Program Running:", SleepSuppress::NONE)
#ifdef PA_ENABLE_SLEEP_SUPPRESS_NO_SLEEP
    , PROGRAM_RUNNING("Program is Running:", SleepSuppress::NO_SLEEP)
#else
    , PROGRAM_RUNNING("Program is Running:", SleepSuppress::NONE)
#endif
    , m_idle_scope(IDLE)
{
    PA_ADD_OPTION(IDLE);
    PA_ADD_OPTION(PROGRAM_RUNNING);

    IDLE.add_listener(*this);
    PROGRAM_RUNNING.add_listener(*this);
}

void SleepSuppressOptions::on_config_value_changed(void* object){
    if (object == &IDLE){
        m_idle_scope = IDLE;
        PROGRAM_RUNNING.set_value(std::max(IDLE.current_value(), PROGRAM_RUNNING.current_value()));
    }else{
        IDLE.set_value(std::min(IDLE.current_value(), PROGRAM_RUNNING.current_value()));
    }
}


}

















