/*  Generate Pokemon Image Training Data
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  Starting at Jubilife Village or any camp, talk to npc to get pokemon from box to party,
 *  throw the pokemon out onto the field to take screenshots with it, move around it to take
 *  screenshots from various angles. Repeat for rest of the pokemon in specified boxes.
 */

#ifndef PokemonAutomation_PokemonLA_GeneratePokemonImageTrainingData_H
#define PokemonAutomation_PokemonLA_GeneratePokemonImageTrainingData_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/PokemonLA_Locations.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

struct TravelLocation;

enum class StartingLocation{
    JUBILIFE_VILLAGE,
    FIELDLANDS_FIELDLANDS,
    FIELDLANDS_HEIGHTS,
    MIRELANDS_MIRELANDS,
    MIRELANDS_BOGBOUND,
    COASTLANDS_BEACHSIDE,
    COASTLANDS_COASTLANDS,
    HIGHLANDS_HIGHLANDS,
    HIGHLANDS_MOUNTAIN,
    HIGHLANDS_SUMMIT,
    ICELANDS_SNOWFIELDS,
    ICELANDS_ICEPEAK,
};

const TravelLocation& to_travel_location(StartingLocation location);

class GeneratePokemonImageTrainingData_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GeneratePokemonImageTrainingData_Descriptor();
};


class GeneratePokemonImageTrainingData : public SingleSwitchProgramInstance{
public:
    GeneratePokemonImageTrainingData();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    // Helper methods
    void select_starting_location(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void talk_to_box_npc(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // Location selection
    EnumDropdownOption<StartingLocation> STARTING_LOCATION;

    SimpleIntegerOption<uint16_t> NUM_BOXES;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};




}
}
}
#endif