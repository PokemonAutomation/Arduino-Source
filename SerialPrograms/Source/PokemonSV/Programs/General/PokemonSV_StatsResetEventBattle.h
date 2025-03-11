/*  Stats Reset Event Battle
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_StatsResetEventBattle_H
#define PokemonAutomation_PokemonSV_StatsResetEventBattle_H

#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


class IvDisplay : public GroupOption{
public:
    IvDisplay();

    void set(const IvRanges& ivs);

private:
    static std::string get_range_string(const IvRange& range);

public:
    StringOption hp;
    StringOption atk;
    StringOption def;
    StringOption spatk;
    StringOption spdef;
    StringOption speed;
};


class StatsResetEventBattle_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StatsResetEventBattle_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class StatsResetEventBattle : public SingleSwitchProgramInstance{
public:
    StatsResetEventBattle();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    bool check_stats_after_win(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    enum class Target{
        Ursaluna,
        Pecharunt,
    };
    EnumDropdownOption<Target> TARGET;

    IvDisplay CALCULATED_IVS;

    OCR::LanguageOCROption LANGUAGE;
    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    BooleanCheckBoxOption TRY_TO_TERASTILLIZE;

//    Pokemon::StatsHuntIvJudgeFilterTable FILTERS;
    Pokemon::StatsHuntIvRangeFilterTable FILTERS0;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    void enter_battle_ursaluna(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void enter_battle_pecharunt(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    bool run_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    bool check_stats(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
};

}
}
}
#endif
