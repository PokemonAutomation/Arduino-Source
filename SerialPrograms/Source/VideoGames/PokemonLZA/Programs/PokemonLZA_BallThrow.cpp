/*  Ball Throw
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stddef.h>
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonLZA_BallThrow.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace std::chrono_literals;



namespace{

//  The D-pad presses that scroll `right_scrolls` balls: right when positive,
//  left when negative.
struct BallScrolls{
    DpadPosition direction;
    size_t count;
};

BallScrolls ball_scrolls(int8_t right_scrolls){
    if (right_scrolls >= 0){
        return {DPAD_RIGHT, (size_t)right_scrolls};
    }
    return {DPAD_LEFT, (size_t)(-(int)right_scrolls)};
}

//  How long ZL + ZR stay held: the aim plus every scroll.
Milliseconds throw_buttons_hold(const BallThrowOptions& options){
    const BallScrolls scrolls = ball_scrolls(options.right_scrolls);
    return options.aim + (options.scroll_hold + options.scroll_release) * (Milliseconds::rep)scrolls.count;
}

}  // namespace


void throw_ball(ProControllerContext& context, const BallThrowOptions& options){
    if (options.lock_on > 0ms){
        pbf_controller_state(
            context, BUTTON_ZL, DPAD_NONE,
            /*left_joystick=*/{0, 0}, /*right_joystick=*/{0, 0},
            /*duration=*/options.lock_on
        );
    }

    //  Use ssf_press_button (non-blocking) instead of pbf_press_button so
    //  ZL + ZR remain held while the D-pad ball-scroll loop runs after `aim`.
    //  With zero scrolls + pbf_wait(after_throw), this matches
    //  pbf_press_button(ZL | ZR, aim, after_throw).
    ssf_press_button(
        context,
        BUTTON_ZL | BUTTON_ZR,
        /*delay=*/options.aim,
        /*hold=*/throw_buttons_hold(options),
        /*cool=*/0ms
    );
    const BallScrolls scrolls = ball_scrolls(options.right_scrolls);
    for (size_t c = 0; c < scrolls.count; c++){
        pbf_press_dpad(context, scrolls.direction, options.scroll_hold, options.scroll_release);
    }

    if (options.after_throw > 0ms){
        pbf_wait(context, options.after_throw);
    }
}



}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
