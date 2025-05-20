/*  Misc. Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_MiscOptions_H
#define PokemonAutomation_PokemonLA_MiscOptions_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "PokemonLA/PokemonLA_WeatherAndTime.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


enum class ResetMethod{
    SoftReset,
    GoBackToVillage,
};

class ResetMethodOption : public EnumDropdownOption<ResetMethod>{
public:
    ResetMethodOption()
        : EnumDropdownOption<ResetMethod>(
            "<b>Reset Method:</b>",
            {
                {ResetMethod::SoftReset,        "reset",                "Soft Reset"},
                {ResetMethod::GoBackToVillage,  "return-to-village",    "Go back to village"},
            },
            LockMode::UNLOCK_WHILE_RUNNING,
            ResetMethod::SoftReset
        )
    {}
};





enum class StopOn{
    Shiny,
    Alpha,
    ShinyOrAlpha,
    ShinyAndAlpha,
};

class StopOnOption : public EnumDropdownOption<StopOn>{
public:
    StopOnOption()
        : EnumDropdownOption<StopOn>(
            "<b>Stop On:</b>",
            {
                {StopOn::Shiny,         "shiny",            "Shiny"},
                {StopOn::Alpha,         "alpha",            "Alpha"},
                {StopOn::ShinyOrAlpha,  "shiny-or-alpha",   "Shiny or Alpha"},
                {StopOn::ShinyAndAlpha, "shiny-and-alpha",  "Shiny and Alpha"},
            },
            LockMode::UNLOCK_WHILE_RUNNING,
            StopOn::ShinyOrAlpha
        )
    {}
};



enum class ExitBattleMethod{
    RunAway,
    MashAToKill,
};

class ExitBattleMethodOption : public EnumDropdownOption<ExitBattleMethod>{
public:
    ExitBattleMethodOption()
        : EnumDropdownOption<ExitBattleMethod>(
            "<b>Exit Battle Method:</b>",
            {
                {ExitBattleMethod::RunAway, "run", "Run Away"},
                {ExitBattleMethod::MashAToKill, "mash-a-to-kill", "Mash A to Kill"},
            },
            LockMode::UNLOCK_WHILE_RUNNING,
            ExitBattleMethod::RunAway
        )
    {}
};


class TimeOfDayOption : public EnumDropdownOption<TimeOfDay>{
public:
    TimeOfDayOption(std::string label = "<b>Time of Day:</b>")
        : EnumDropdownOption<TimeOfDay>(
            label,
            {
                {TimeOfDay::NONE,     TIME_OF_DAY_NAMES[int(TimeOfDay::NONE)], "None"},
                {TimeOfDay::MORNING,  TIME_OF_DAY_NAMES[int(TimeOfDay::MORNING)], "Morning"},
                {TimeOfDay::MIDDAY,   TIME_OF_DAY_NAMES[int(TimeOfDay::MIDDAY)], "Midday"},
                {TimeOfDay::EVENING,  TIME_OF_DAY_NAMES[int(TimeOfDay::EVENING)], "Evening"},
                {TimeOfDay::MIDNIGHT, TIME_OF_DAY_NAMES[int(TimeOfDay::MIDNIGHT)], "Midnight"},
            },
            LockMode::UNLOCK_WHILE_RUNNING,
            TimeOfDay::NONE
        )
    {}
};



}
}
}
#endif
