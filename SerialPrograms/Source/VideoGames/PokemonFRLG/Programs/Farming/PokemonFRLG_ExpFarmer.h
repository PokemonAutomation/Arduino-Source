/*  EXP Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Farm EXP by rebattling trainers with the VS Seeker.
 *  Seven Island: Cooltrainers Leroy and Michelle near the end of Sevault
 *  Canyon, healing at the Chansey dance house next to them.
 *  One Island: Black Belts Hugh and Shea, Crush Kin Mik & Kia (a double
 *  battle) and Crush Girl Sharon on Kindle Road, healing at the Ember Spa.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_ExpFarmer_H
#define PokemonAutomation_PokemonFRLG_ExpFarmer_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/ButtonOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


class ExpFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ExpFarmer_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ExpFarmer : public SingleSwitchProgramInstance{
public:
    using Descriptor = ExpFarmer_Descriptor;
    ExpFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

    enum class EvolutionChoice{
        CANCEL,
        ALLOW,
        ALLOW_AND_STOP,
    };

    enum class MoveLearnChoice{
        DECLINE,
        REPLACE_SLOT_4,
        STOP,
    };

    enum class Location{
        ONE_ISLAND,
        SEVEN_ISLAND,
    };

    //  Which Pokemon the target level is for.
    enum class LevelingTarget{
        BATTLING,   //  The lead: slot 1, or the switch-in Pokemon.
        EXP_SHARE,  //  A Pokemon that doesn't battle (holds an Exp. Share).
    };

private:
    void run_seven_island(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void run_one_island(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    DeferredStopButtonOption STOP_AFTER_CURRENT;
    EnumDropdownOption<Location> LOCATION;
    BooleanCheckBoxOption SWITCH_IN;
    EnumDropdownOption<EvolutionChoice> EVOLUTION;
    EnumDropdownOption<MoveLearnChoice> MOVE_LEARN;
    SimpleIntegerOption<uint8_t> MAX_REPEL_SLOT;
    EnumDropdownOption<LevelingTarget> LEVELING;
    SimpleIntegerOption<uint8_t> TARGET_LEVEL;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
