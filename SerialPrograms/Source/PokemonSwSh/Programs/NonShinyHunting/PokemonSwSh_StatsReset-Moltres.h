/*  Stats Reset - Moltres
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsResetMoltres_H
#define PokemonAutomation_PokemonSwSh_StatsResetMoltres_H

#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Pokemon/Options/Pokemon_IvJudgeOption.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StatsResetMoltres_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StatsResetMoltres_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class StatsResetMoltres : public SingleSwitchProgramInstance{
public:
    StatsResetMoltres();
    virtual void program(SingleSwitchProgramEnvironment& env, ControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCROption LANGUAGE;
    IVJudgeFilterOption HP;
    IVJudgeFilterOption ATTACK;
    IVJudgeFilterOption DEFENSE;
    IVJudgeFilterOption SPATK;
    IVJudgeFilterOption SPDEF;
    IVJudgeFilterOption SPEED;

    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
