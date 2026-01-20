/*  Stats Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_StatsReset_H
#define PokemonAutomation_PokemonLZA_StatsReset_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_IvJudgeOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;


class StatsReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StatsReset_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class StatsReset : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    StatsReset();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    virtual void on_config_value_changed(void* object) override;
    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCROption LANGUAGE;

    enum class GiftPokemon{
        FLOETTE,
        MAGEARNA,
        MELTAN,
        MELMETAL,
    };
    EnumDropdownOption<GiftPokemon> POKEMON;

    SimpleIntegerOption<int8_t> RIGHT_SCROLLS;
    MillisecondsOption SCROLL_HOLD;
    MillisecondsOption SCROLL_RELEASE;
    MillisecondsOption POST_THROW_WAIT;

    IVJudgeFilterOption HP;
    IVJudgeFilterOption ATTACK;
    IVJudgeFilterOption DEFENSE;
    IVJudgeFilterOption SPATK;
    IVJudgeFilterOption SPDEF;
    IVJudgeFilterOption SPEED;

    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
