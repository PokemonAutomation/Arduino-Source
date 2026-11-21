/*  Ball Throw
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Throw a ball at the Pokemon the player faces: lock on with ZL, hold ZR
 *  with it to raise a ball, optionally scroll to another ball with the
 *  D-pad while both are held, then let go to throw.
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BallThrow_H
#define PokemonAutomation_PokemonLZA_BallThrow_H

#include <stdint.h>
#include "Common/Cpp/Time.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation{

namespace NintendoSwitch{
namespace PokemonLZA{



//  The presses and timings of one throw (throw_ball()).
struct BallThrowOptions{
    //  ZL alone before ZR joins it, to lock on first. 0 presses both at once.
    Milliseconds lock_on = Milliseconds(0);

    //  ZL + ZR held before the first scroll, or before the throw when there
    //  is no scroll.
    Milliseconds aim = Milliseconds(500);

    //  Balls to scroll to the right while aiming. Negative scrolls left.
    int8_t right_scrolls = 0;
    Milliseconds scroll_hold = Milliseconds(120);
    Milliseconds scroll_release = Milliseconds(80);

    //  Wait after letting go of ZL + ZR.
    Milliseconds after_throw = Milliseconds(0);
};


//  Lock on, aim, scroll and throw as `options` says, then wait
//  `options.after_throw`. Presses only: the caller checks what the throw did.
void throw_ball(ProControllerContext& context, const BallThrowOptions& options);



}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
#endif
