/*  Stats Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_StatsReset_H
#define PokemonAutomation_PokemonSwSh_StatsReset_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/ScreenshotFormatOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_IVCheckerOption.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IVCheckerReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class StatsReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    StatsReset_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class StatsReset : public SingleSwitchProgramInstance{
public:
    StatsReset();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCROption LANGUAGE;

    enum class GiftPokemon{
        TypeNull,
        Cosmog,
        Poipole,
    };
    EnumDropdownOption<GiftPokemon> POKEMON;

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
