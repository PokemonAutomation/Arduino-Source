/*  Max Lair (Single Adventure Run)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_SingleRun_H
#define PokemonAutomation_PokemonSwSh_MaxLair_SingleRun_H

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


class MaxLairSingleRun_Descriptor : public MultiSwitchProgramDescriptor{
public:
    MaxLairSingleRun_Descriptor();
};


class MaxLairSingleRun : public MultiSwitchProgramInstance{
public:
    MaxLairSingleRun(const MaxLairSingleRun_Descriptor& descriptor);

    virtual bool is_valid() const override;

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(MultiSwitchProgramEnvironment& env) override;


private:


private:
    EnumDropdownOption HOST_SWITCH;
    EnumDropdownOption BOSS_SLOT;
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
