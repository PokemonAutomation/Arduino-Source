/*  Stats Reset - Moltres
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsResetMoltres_H
#define PokemonAutomation_PokemonSwSh_StatsResetMoltres_H

#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "CommonFramework/OCR/LanguageOptionOCR.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Options/GoHomeWhenDone.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IVCheckerReader.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StatsResetMoltres_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    StatsResetMoltres_Descriptor();
};



class StatsResetMoltres : public SingleSwitchProgramInstance{
public:
    StatsResetMoltres(const StatsResetMoltres_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    struct Stats;

    StartInGripOrGame START_IN_GRIP_MENU;
    GoHomeWhenDone GO_HOME_WHEN_DONE;

    IVCheckerReader m_iv_checker_reader;

    OCR::LanguageOCR LANGUAGE;
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
