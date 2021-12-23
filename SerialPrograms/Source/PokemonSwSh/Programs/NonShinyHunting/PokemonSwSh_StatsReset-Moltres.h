/*  Stats Reset - Moltres
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsResetMoltres_H
#define PokemonAutomation_PokemonSwSh_StatsResetMoltres_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "Pokemon/Options/Pokemon_IVCheckerOption.h"
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

    StartInGripOrGameOption START_IN_GRIP_MENU;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCR LANGUAGE;
    IVCheckerFilterOption HP;
    IVCheckerFilterOption ATTACK;
    IVCheckerFilterOption DEFENSE;
    IVCheckerFilterOption SPATK;
    IVCheckerFilterOption SPDEF;
    IVCheckerFilterOption SPEED;

    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
