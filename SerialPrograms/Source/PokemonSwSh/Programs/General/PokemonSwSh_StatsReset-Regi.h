/*  Stats Reset - Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsResetRegi_H
#define PokemonAutomation_PokemonSwSh_StatsResetRegi_H

#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "CommonFramework/OCR/LanguageOptionOCR.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Options/GoHomeWhenDone.h"
#include "Pokemon/Options/Pokemon_BallSelect.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IVCheckerReader.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StatsResetRegi_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    StatsResetRegi_Descriptor();
};



class StatsResetRegi : public SingleSwitchProgramInstance{
public:
    StatsResetRegi(const StatsResetRegi_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    struct Stats;

    StartInGripOrGame START_IN_GRIP_MENU;
    GoHomeWhenDone GO_HOME_WHEN_DONE;

    IVCheckerReader m_iv_checker_reader;

    Pokemon::PokemonBallSelect BALL_SELECT;
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
