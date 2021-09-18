/*  Max Lair (Standard Mode)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Standard_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Standard_H

#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Options/FixedCodeOption.h"
#include "CommonFramework/OCR/LanguageOptionOCR.h"
#include "NintendoSwitch/Framework/MultiSwitchProgram.h"
#include "Framework/PokemonSwSh_MaxLair_Options.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;


class MaxLairStandard_Descriptor : public MultiSwitchProgramDescriptor{
public:
    MaxLairStandard_Descriptor();
};


class MaxLairStandard : public MultiSwitchProgramInstance{
public:
    MaxLairStandard(const MaxLairStandard_Descriptor& descriptor);

    virtual bool is_valid() const override;

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(MultiSwitchProgramEnvironment& env) override;


private:
    EnumDropdownOption HOST_SWITCH;
    EnumDropdownOption BOSS_SLOT;
    EnumDropdownOption STOP_CONDITION;
    FixedCodeOption RAID_CODE;

    ScreenshotOption SCREENSHOT;

    LanguageSet m_languages;

    MaxLairPlayerOptions PLAYER0;
    MaxLairPlayerOptions PLAYER1;
    MaxLairPlayerOptions PLAYER2;
    MaxLairPlayerOptions PLAYER3;
};



}
}
}
#endif
