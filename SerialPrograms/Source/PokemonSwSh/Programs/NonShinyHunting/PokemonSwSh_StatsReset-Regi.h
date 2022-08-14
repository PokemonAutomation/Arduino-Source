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
#include "Pokemon/Options/Pokemon_BallSelectOption.h"
#include "Pokemon/Options/Pokemon_IVCheckerOption.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IVCheckerReader.h"
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
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    PokemonBallSelect BALL_SELECT;
    OCR::LanguageOCR LANGUAGE;
    IVCheckerFilterOption HP;
    IVCheckerFilterOption ATTACK;
    IVCheckerFilterOption DEFENSE;
    IVCheckerFilterOption SPATK;
    IVCheckerFilterOption SPDEF;
    IVCheckerFilterOption SPEED;

    EventNotificationOption NOTIFICATION_CATCH_SUCCESS;
    EventNotificationOption NOTIFICATION_CATCH_FAILED;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
