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

protected:
    //  Call first in program(), with the game on the overworld where the
    //  player saved: checks the party has the free slots
    //  free_party_slots_needed() asks for, then goes Home for the black
    //  border check. Throws UserSetupError if a check fails.
    void run_precheck(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context
    );

private:
    void check_free_party_slots(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context
    );
};


}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
