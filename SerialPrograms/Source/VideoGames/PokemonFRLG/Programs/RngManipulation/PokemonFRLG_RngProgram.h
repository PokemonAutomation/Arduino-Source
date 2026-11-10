/*  RNG Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Base class for the FRLG RNG programs. Each program states what the game
 *  must look like when it starts, and the base class checks it before the
 *  program runs, so a setup mistake stops the run at once instead of failing
 *  in a misleading way many resets later.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RngProgram_H
#define PokemonAutomation_PokemonFRLG_RngProgram_H

#include <cstddef>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "VideoGames/PokemonFRLG/Inference/Menus/PokemonFRLG_PartySlot.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


//  The game steps that check_party_free_slots() takes. They are injected so
//  that its decisions run in tests without a Switch.
class PartyCheckActions{
public:
    virtual ~PartyCheckActions() = default;

    //  Open the party menu from the overworld. Returns false if it can't.
    virtual bool open_party_menu() = 0;

    //  Read the last occupied slot on the open party menu.
    virtual PartySlot read_last_occupied_slot() = 0;
};

//  Checks that the party has at least `free_slots_needed` free slots,
//  starting from the overworld. Throws UserSetupError if it doesn't. If the
//  party menu can't be opened, logs that and returns. May leave the party
//  menu open.
void check_party_free_slots(
    Logger& logger,
    size_t free_slots_needed,
    PartyCheckActions& actions
);


//  Base class of the RNG programs. Before program() runs, it checks that the
//  party has the free slots the program needs.
class RngProgramInstance : public SingleSwitchProgramInstance{
public:
    using SingleSwitchProgramInstance::SingleSwitchProgramInstance;

    //  How many free party slots the program needs: one for each Pokemon it
    //  catches or receives and then reads from the end of the party. In a full
    //  party that Pokemon would go to the PC instead.
    virtual size_t free_party_slots_needed() const = 0;

    //  The start menu the player sees where the program starts.
    virtual StartMenuContext start_menu_context() const{
        return StartMenuContext::STANDARD;
    }

    //  Runs with the game on the overworld where the player saved. May leave a
    //  menu open: the program's first reset closes the game. Final so that
    //  every RNG program runs the check free_party_slots_needed() asks for.
    virtual void start_program_game_checks(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context
    ) override final;
};


}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
