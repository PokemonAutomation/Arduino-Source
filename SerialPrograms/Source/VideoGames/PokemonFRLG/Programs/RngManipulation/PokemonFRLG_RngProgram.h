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
#include "NintendoSwitch/Programs/NintendoSwitch_Prechecks.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


//  The prechecks of an RNG program that needs free_party_slots_needed free
//  party slots: black_border, then free_party_slots when it needs any.
PrecheckList rng_prechecks(size_t free_party_slots_needed);


//  Base class of the RNG programs.
class RngProgramInstance : public SingleSwitchProgramInstance{
public:
    using SingleSwitchProgramInstance::SingleSwitchProgramInstance;

    //  How many free party slots the program needs: one for each Pokemon it
    //  catches or receives and then reads from the end of the party. In a full
    //  party that Pokemon would go to the PC instead.
    virtual size_t free_party_slots_needed() const = 0;

    //  What the program needs before it starts, in the order run_precheck()
    //  checks it. Does not touch the console.
    PrecheckList prechecks() const;

protected:
    //  Call first in program(), with the game on the overworld where the
    //  player saved: runs prechecks(). "Run Start Checks" off in the FRLG Game
    //  Settings skips the optional ones. Throws UserSetupError if a check fails.
    void run_precheck(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context
    );
};


}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
