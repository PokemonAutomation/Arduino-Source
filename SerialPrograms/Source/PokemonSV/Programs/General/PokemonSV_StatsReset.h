/*  Stats Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_StatsReset_H
#define PokemonAutomation_PokemonSV_StatsReset_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"
#include "PokemonSV/Options/PokemonSV_BattleMoveTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class StatsReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StatsReset_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class StatsReset : public SingleSwitchProgramInstance{
public:
    StatsReset();
    virtual void program(SingleSwitchProgramEnvironment& env, ControllerContext& context) override;

private:
    //Can expand targets assuming there's anything else not locked in the DLC
    enum class Target{
        TreasuresOfRuin,
        LoyalThree,
        Snacksworth,
        Generic,
    };
    EnumDropdownOption<Target> TARGET;

    OCR::LanguageOCROption LANGUAGE;
    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    BooleanCheckBoxOption QUICKBALL;
    BattleMoveTable BATTLE_MOVES;
    Pokemon::StatsHuntIvJudgeFilterTable FILTERS;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    bool enter_battle(SingleSwitchProgramEnvironment& env, ControllerContext& context);
    void open_ball_menu(SingleSwitchProgramEnvironment& env, ControllerContext& context);
    bool run_battle(SingleSwitchProgramEnvironment& env, ControllerContext& context);
    bool check_stats(SingleSwitchProgramEnvironment& env, ControllerContext& context);
};

}
}
}
#endif
