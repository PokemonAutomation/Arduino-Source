/*  Shiny Hunt - Terrakion
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyHunt_Terrakion_H
#define PokemonAutomation_PokemonLZA_ShinyHunt_Terrakion_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


class ShinyHunt_Terrakion_Descriptor : public SingleSwitchProgramDescriptor {
public:
    ShinyHunt_Terrakion_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_Terrakion : public SingleSwitchProgramInstance {
public:
    ShinyHunt_Terrakion();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;
    SimpleIntegerOption<uint16_t> THRESHOLD;
    BooleanCheckBoxOption SAVE_ON_START;
    ShinySoundDetectedActionOption SHINY_DETECTED;
    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
