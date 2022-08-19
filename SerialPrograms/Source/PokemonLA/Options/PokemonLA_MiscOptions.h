/*  Misc. Options
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_MiscOptions_H
#define PokemonAutomation_PokemonLA_MiscOptions_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

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
            ExitBattleMethod::RunAway
        )
    {}
};



}
}
}
#endif
