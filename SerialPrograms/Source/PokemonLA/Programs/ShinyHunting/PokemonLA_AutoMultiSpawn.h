/*  Auto Multi-Spawn
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  Advance a path in Multi-Spawn shiny hunting method
 */

#ifndef PokemonAutomation_PokemonLA_AutoMultiSpawn_H
#define PokemonAutomation_PokemonLA_AutoMultiSpawn_H

#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/PokemonLA_WeatherAndTime.h"
#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class AutoMultiSpawn_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoMultiSpawn_Descriptor();
};


enum class MultiSpawn{
    MirelandsHippopotas,
};


class AutoMultiSpawn : public SingleSwitchProgramInstance{
public:
    AutoMultiSpawn();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    // Advance one step (e.g. A1 or A2) in the multi-spawn path
    void advance_one_path_step(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        size_t num_spawned_pokemon,
        size_t num_to_despawn,
        TimeOfDay cur_time,
        TimeOfDay next_time
    );

    // From camp, go to the spawn point, start one battle to remove some pokemon
    // Return how many pokemon removed in the battle.
    // The function will use a loop to do multiple trips to the spawn point if it cannot find a target pokemon after a trip.
    size_t try_one_battle_to_remove_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t num_pokemon_left, size_t num_to_despawn);

    // From camp go to the spawn point, try focusing on one pokemon
    // Return the pokemon details if found the target pokemon. Otherwise if cannot find one, return empty details
    // The function will use a loop to do multiple focusing to try to get a target pokemon focused
    PokemonDetails go_to_spawn_point_and_try_focusing_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t nun_pokemon_left);

    OCR::LanguageOCROption LANGUAGE;

    EnumDropdownOption<MultiSpawn> SPAWN;

    StringOption PATH;

    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
