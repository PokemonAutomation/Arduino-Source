/*  RNG Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Base class for the FRLG RNG programs. Each program states what the game
 *  must look like when it starts, and checks it first thing, so a setup
 *  mistake stops the run at once instead of failing in a misleading way many
 *  resets later.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RngProgram_H
#define PokemonAutomation_PokemonFRLG_RngProgram_H

#include <cstddef>
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


//  Base class of the RNG programs.
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

protected:
    //  Checks that the party has the free slots free_party_slots_needed()
    //  asks for; throws UserSetupError if not. Call it first in program(),
    //  with the game on the overworld where the player saved. May leave the
    //  party menu open: the program's first reset closes the game.
    void check_free_party_slots(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context
    );
};


}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
