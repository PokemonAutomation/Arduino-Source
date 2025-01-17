/*  Tree Leap Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_LeapGrinder_H
#define PokemonAutomation_PokemonLA_LeapGrinder_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_MiscOptions.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/PokemonLA_UnderAttackDetector.h"
#include "PokemonLA/Programs/PokemonLA_LeapPokemonActions.h"
#include "Pokemon/Options/Pokemon_NameSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class LeapGrinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    LeapGrinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class LeapGrinder : public SingleSwitchProgramInstance{
public:
    LeapGrinder();
    virtual void program(SingleSwitchProgramEnvironment& env, ControllerContext& context) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env, ControllerContext& context);
    bool quick_check(SingleSwitchProgramEnvironment& env, ControllerContext& context);

private:
    class RunRoute;

    OCR::LanguageOCROption LANGUAGE;

    StringSelectDatabase POKEMON_DATABASE;
    StringSelectOption POKEMON;

    SimpleIntegerOption<uint8_t> LEAPS;
    StopOnOption STOP_ON;
    ExitBattleMethodOption EXIT_METHOD;
    ShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    ShinyDetectedActionOption MATCH_DETECTED_OPTIONS;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
