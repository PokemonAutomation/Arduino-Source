/*  Auto Multi-Spawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  Advance a path in Multi-Spawn shiny hunting method
 */

#ifndef PokemonAutomation_PokemonLA_AutoMultiSpawn_H
#define PokemonAutomation_PokemonLA_AutoMultiSpawn_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/StringOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/PokemonLA_WeatherAndTime.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class AutoMultiSpawn_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoMultiSpawn_Descriptor();
};


class AutoMultiSpawn : public SingleSwitchProgramInstance{
public:
    AutoMultiSpawn();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    // Advance one step (e.g. A1 or A2) in the multi-spawn path
    void advance_one_path_step(SingleSwitchProgramEnvironment& env, BotBaseContext& context, size_t num_to_despawn, TimeOfDay cur_time, TimeOfDay next_time);

    // Go to the spawn point, start one battle to remove some pokemon
    // Return how many pokemon removed in the battle
    size_t one_battle_to_remove_pokemon(SingleSwitchProgramEnvironment& env, BotBaseContext& context, size_t num_to_despawn);

    OCR::LanguageOCR LANGUAGE;

    EnumDropdownOption SPAWN;

    StringOption PATH;

    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
