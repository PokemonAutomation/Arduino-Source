/*  Sleep Suppress Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SleepSuppressOption_H
#define PokemonAutomation_SleepSuppressOption_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "CommonFramework/Environment/SystemSleep.h"

namespace PokemonAutomation{



class SleepSuppressOption : public EnumDropdownOption<SleepSuppress>{
public:
    SleepSuppressOption(std::string label, SleepSuppress default_value);
};


class SleepSuppressOptions : public GroupOption, public ConfigOption::Listener{
public:
    ~SleepSuppressOptions();
    SleepSuppressOptions();

    virtual void on_config_value_changed(void* object) override;

public:
    SleepSuppressOption IDLE;
    SleepSuppressOption PROGRAM_RUNNING;

private:
    SleepSuppressScope m_idle_scope;
};


}
#endif
