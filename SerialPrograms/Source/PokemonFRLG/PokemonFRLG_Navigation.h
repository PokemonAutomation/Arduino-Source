/*  Pokemon FRLG Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Soft reset, menus, etc.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_Navigation_H
#define PokemonAutomation_PokemonFRLG_Navigation_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
    class ConsoleHandle;
    class ProController;
    using ProControllerContext = ControllerContext<ProController>;
namespace PokemonFRLG{

enum class BattleResult{
    opponentfainted,
    playerfainted,
    outofpp,
    unknown
};

enum class KantoFlyLocation{
    pallettown,
    viridiancity,
    pewtercity,
    route4,
    ceruleancity,
    vermilioncity,
    route10,
    lavendertown,
    celadoncity,
    saffroncity,
    fuschiacity,
    cinnabarisland,
    indigoplateau,
};


// Press A+B+Select+Start at the same time to soft reset, then re-enters the game.
// There are two random waits, one before pressing start and another after loading in the game.
// This is to prevent repeatedly getting the same pokemon, due to FRLG's RNG.
uint64_t soft_reset(ConsoleHandle& console, ProControllerContext &context);

// From the overworld, open the summary of the Pokemon in slot 6. This assumes the menu cursor is in the top slot (POKEDEX)
uint64_t open_slot_six(ConsoleHandle& console, ProControllerContext& context);

// After press A/walking up to enter a battle, run this handle the battle start and to check if opponent is shiny.
// Set send_out_lead to true and then use flee_battle() after if for run away resets
// For soft resets, send_out_lead as false and then soft_reset() to save time.
bool handle_encounter(ConsoleHandle& console, ProControllerContext& context, bool send_out_lead);

// Mash A to keep using the first move until a Pokemon faints (either the player's or the opponent)
// Flees the battle if out of PP.
// Returns a BattleResult indicating how the battle ended
BattleResult spam_first_move(ConsoleHandle& console, ProControllerContext& context);

// Run from battle. Cursor must start on the FIGHT button. Assumes fleeing will always work. (Smoke Ball)
void flee_battle(ConsoleHandle& console, ProControllerContext& context);

// Exit a wild battle after winning. Checks if a Pokemon is learning a new move.
// If stop_on_move_learn is true, this exits early when a move is being learned without declining it. Otherwise, this returns to the overworld. 
// Returns true if a move was learned (even if it was rejected) and false otherwise.
bool exit_wild_battle(ConsoleHandle& console, ProControllerContext& context, bool stop_on_move_learn, bool prevent_evolution);

// Starting from the start menu, a sub-screen of the start menu, or the overworld, navigate to the party screen
enum class StartMenuContext {
    STANDARD,
    SAFARI_ZONE
};
void open_party_menu_from_overworld(ConsoleHandle& console, ProControllerContext& context, StartMenuContext menu_context = StartMenuContext::STANDARD);

// Starting from the start menu, a sub-screen of the start menu, or the overworld, navigate to the bag
void open_bag_from_overworld(ConsoleHandle& console, ProControllerContext& context, StartMenuContext menu_context = StartMenuContext::STANDARD);

// Uses Teleport to return to a PokeCenter. 
// Assumes that Teleport is usable and the last party member has it learned
void use_teleport_from_overworld(ConsoleHandle& console, ProControllerContext& context);

// Navigates to the fly map. Assumes that Fly is usable and the last member of your party has it learned
void open_fly_map_from_overworld(ConsoleHandle& console, ProControllerContext& context);

// Starting from the Kanto Fly map, fly to a specified location.
void fly_from_kanto_map(ConsoleHandle& console, ProControllerContext& context, KantoFlyLocation destination);

// Enter a PokeCenter. Assumes the player is standing in front of its door
void enter_pokecenter(ConsoleHandle& console, ProControllerContext& context);

// Leave a PokeCenter. Assumes the player is standing directly north of the exit
void leave_pokecenter(ConsoleHandle& console, ProControllerContext& context);

// Approach the counter and heal at a PokeCenter. Assumed the player is directly south of the nurse
// Combine with enter_pokecenter, leave_pokecenter, and use_teleport_from_overworld for automating healing your party
void heal_at_pokecenter(ConsoleHandle& console, ProControllerContext& context);

// Trigger encounters in grass without moving by tapping the left thumbstick back and forth
// Can be used to alternate left/right and up/down. It is important that the player is not facing
// the same direction as the first thumbstick press.
// returns -1 if no encounter is triggered, 0 if a non-shiny is encounter, and 1 if a shiny is encountered
int grass_spin(ConsoleHandle& console, ProControllerContext& context, bool leftright, Seconds timeout = 60s);

// Go to home to check that scaling is 100%. Then resume game.
void home_black_border_check(ConsoleHandle& console, ProControllerContext& context);

}
}
}
#endif
