/*  Elite Four Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Repeatedly beat the rematch Elite Four and Champion with a single
 *  Pokemon that one-hit KOs every opponent.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_EliteFourFarmer_H
#define PokemonAutomation_PokemonFRLG_EliteFourFarmer_H

#include "Common/Cpp/Options/ButtonOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


class EliteFourFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EliteFourFarmer_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class EliteFourFarmer : public SingleSwitchProgramInstance{
public:
    using Descriptor = EliteFourFarmer_Descriptor;
    EliteFourFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

    // The Pokemon doing all the battling. Each needs a specific moveset;
    // see the move tables in the .cpp.
    enum class Attacker{
        STARMIE,
        MEWTWO,
        LAPRAS,
    };

    // The starter the PLAYER chose. This decides the rival's team.
    enum class Starter{
        BULBASAUR,
        CHARMANDER,
        SQUIRTLE,
    };

private:
    DeferredStopButtonOption STOP_AFTER_CURRENT;
    EnumDropdownOption<Attacker> ATTACKER;
    EnumDropdownOption<Starter> STARTER;
    SimpleIntegerOption<uint32_t> NUM_WINS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
