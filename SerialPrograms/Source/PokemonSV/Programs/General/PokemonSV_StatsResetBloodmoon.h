/*  Stats Reset Bloodmoon
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_StatsResetBloodmoon_H
#define PokemonAutomation_PokemonSV_StatsResetBloodmoon_H

#include "Common/Cpp/Options/StringOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/LanguageOCROption.h"
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


class StatsResetBloodmoon_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StatsResetBloodmoon_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class StatsResetBloodmoon : public SingleSwitchProgramInstance{
public:
    StatsResetBloodmoon();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    bool check_stats_after_win(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    IvDisplay CALCULATED_IVS;

    OCR::LanguageOCROption LANGUAGE;
    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    BooleanCheckBoxOption TRY_TO_TERASTILLIZE;

//    Pokemon::StatsHuntIvJudgeFilterTable FILTERS;
    Pokemon::StatsHuntIvRangeFilterTable FILTERS0;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    void enter_battle(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    bool run_battle(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    bool check_stats(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
};

}
}
}
#endif
