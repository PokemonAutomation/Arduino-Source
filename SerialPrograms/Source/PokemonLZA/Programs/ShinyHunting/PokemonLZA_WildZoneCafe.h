/*  Shiny Hunt - Wild Zone Cafe
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_WildZoneCafe_H
#define PokemonAutomation_PokemonLZA_WildZoneCafe_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation::NintendoSwitch::PokemonLZA {


class ShinyHunt_WildZoneCafe_Descriptor : public SingleSwitchProgramDescriptor {
public:
    ShinyHunt_WildZoneCafe_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_WildZoneCafe : public SingleSwitchProgramInstance{
public:
    ShinyHunt_WildZoneCafe();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    EnumDropdownOption<Location> CAFE;
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    SimpleIntegerOption<uint64_t> NUM_VISITS;

    ShinySoundDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}  // namespace PokemonAutomation::NintendoSwitch::PokemonLZA
#endif
