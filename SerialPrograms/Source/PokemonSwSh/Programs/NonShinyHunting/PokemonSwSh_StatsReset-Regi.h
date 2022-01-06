/*  Stats Reset - Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsResetRegi_H
#define PokemonAutomation_PokemonSwSh_StatsResetRegi_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "Pokemon/Options/Pokemon_BallSelectOption.h"
#include "Pokemon/Options/Pokemon_IVCheckerOption.h"
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
    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
