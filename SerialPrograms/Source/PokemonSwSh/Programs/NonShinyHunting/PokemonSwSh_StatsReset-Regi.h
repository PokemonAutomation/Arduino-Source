/*  Stats Reset - Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsResetRegi_H
#define PokemonAutomation_PokemonSwSh_StatsResetRegi_H

#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Pokemon/Options/Pokemon_IvJudgeOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StatsResetRegi_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StatsResetRegi_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class StatsResetRegi : public SingleSwitchProgramInstance{
public:
    StatsResetRegi();
    virtual void program(SingleSwitchProgramEnvironment& env, ControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    PokemonBallSelectOption BALL_SELECT;
    OCR::LanguageOCROption LANGUAGE;
    IVJudgeFilterOption HP;
    IVJudgeFilterOption ATTACK;
    IVJudgeFilterOption DEFENSE;
    IVJudgeFilterOption SPATK;
    IVJudgeFilterOption SPDEF;
    IVJudgeFilterOption SPEED;

    EventNotificationOption NOTIFICATION_CATCH_SUCCESS;
    EventNotificationOption NOTIFICATION_CATCH_FAILED;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
