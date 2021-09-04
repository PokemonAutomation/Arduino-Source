/*  Stats Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsReset_H
#define PokemonAutomation_PokemonSwSh_StatsReset_H

#include "CommonFramework/Options/EnumDropdown.h"
#include "CommonFramework/OCR/LanguageOptionOCR.h"
#include "NintendoSwitch/Options/GoHomeWhenDone.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IVCheckerReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StatsReset_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    StatsReset_Descriptor();
};



class StatsReset : public SingleSwitchProgramInstance{
public:
    StatsReset(const StatsReset_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    struct Stats;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
    GoHomeWhenDone GO_HOME_WHEN_DONE;

    IVCheckerReader m_iv_checker_reader;

    OCR::LanguageOCR LANGUAGE;
    EnumDropdown POKEMON;
    IVCheckerOption HP;
    IVCheckerOption ATTACK;
    IVCheckerOption DEFENSE;
    IVCheckerOption SPATK;
    IVCheckerOption SPDEF;
    IVCheckerOption SPEED;
};


}
}
}
#endif
