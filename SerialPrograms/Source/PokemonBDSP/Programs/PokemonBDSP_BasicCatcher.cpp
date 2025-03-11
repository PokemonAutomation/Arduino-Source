/*  Basic Pokemon Catcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_ReceivePokemonDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_ExperienceGainDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_EndBattleDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleBallReader.h"
#include "PokemonBDSP_BasicCatcher.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Returns the # of slots scrolled. Returns -1 if not found.
int move_to_ball(
    const BattleBallReader& reader,
    VideoStream& stream, ProControllerContext& context,
    const std::string& ball_slug,
    bool forward, int attempts, uint16_t delay
){
    std::string first_ball = reader.read_ball(stream.video().snapshot());
    if (first_ball == ball_slug){
        return 0;
    }

    size_t repeat_counter = 0;
    for (int c = 1; c < attempts; c++){
        pbf_press_dpad(context, forward ? DPAD_RIGHT : DPAD_LEFT, 10, delay);
        context.wait_for_all_requests();
        std::string current_ball = reader.read_ball(stream.video().snapshot());
        if (current_ball == ball_slug){
            return c;
        }
        if (current_ball == first_ball){
            repeat_counter++;
            if (repeat_counter == 3){
                return -1;
            }
        }
    }
    return -1;
}


//  Returns the quantity of the ball.
//  Returns -1 if unable to read.
int16_t move_to_ball(
    const BattleBallReader& reader,
    VideoStream& stream, ProControllerContext& context,
    const std::string& ball_slug
){
    //  Search forward at high speed.
    int ret = move_to_ball(reader, stream, context, ball_slug, true, 50, 30);
    if (ret < 0){
        return 0;
    }
    if (ret == 0){
        uint16_t quantity = reader.read_quantity(stream.video().snapshot());
        return quantity == 0 ? -1 : quantity;
    }

    //  Wait a second to let the video catch up.
    pbf_wait(context, TICKS_PER_SECOND);
    context.wait_for_all_requests();

    //  Now try again in reverse at a lower speed in case we overshot.
    //  This will return immediately if we got it right the first time.
    ret = move_to_ball(reader, stream, context, ball_slug, false, 5, TICKS_PER_SECOND);
    if (ret < 0){
        return 0;
    }
    if (ret > 0){
        stream.log("BasicCatcher: Fast ball scrolling overshot by " +
            std::to_string(ret) + " slot(s).", COLOR_RED);
    }
    uint16_t quantity = reader.read_quantity(stream.video().snapshot());
    return quantity == 0 ? -1 : quantity;
}


CatchResults throw_balls(
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const std::string& ball_slug, uint16_t ball_limit
){
    uint16_t balls_used = 0;
    while (true){
        // Test code for checking catch outcome handling: if the wild pokemon fainted:
// #define TEST_WILD_POKEMON_FAINTED
#ifdef TEST_WILD_POKEMON_FAINTED
        pbf_mash_button(context, BUTTON_ZL, TICKS_PER_SECOND);
        context.wait_for_all_requests();
        if (0)
#endif
        {
            BattleBallReader reader(stream, language);

            pbf_press_button(context, BUTTON_X, 20, 105);
            context.wait_for_all_requests();

            const int16_t num_balls = move_to_ball(reader, stream, context, ball_slug);
            if (num_balls < 0){
                stream.log("BasicCatcher: Unable to read quantity of ball " + ball_slug + ".");
            }
            if (num_balls == 0){
                stream.log("BasicCatcher: No ball " + ball_slug +
                    " found in bag or used them all during catching.");
                return {CatchResult::OUT_OF_BALLS, balls_used};
            }

            stream.log(
                "BasicCatcher: Found " + ball_slug + " with amount " +
                std::to_string(num_balls)
            );
            pbf_mash_button(context, BUTTON_ZL, 125);
            context.wait_for_all_requests();
        }
        balls_used++;

        auto start = current_time();

        BattleMenuWatcher menu_detector(BattleType::STANDARD);
        ExperienceGainWatcher experience_detector;
        SelectionArrowFinder own_fainted_detector(stream.overlay(), {0.18, 0.64, 0.46, 0.3}, COLOR_YELLOW);
        int result = wait_until(
            stream, context,
            std::chrono::seconds(60),
            {
                {menu_detector},
                {experience_detector},
                {own_fainted_detector},
            }
        );
        switch (result){
        case 0:
            if (current_time() < start + std::chrono::seconds(5)){
                return {CatchResult::CANNOT_THROW_BALL, balls_used};
            }
            stream.log("BasicCatcher: Failed to catch.", COLOR_ORANGE);
            if (balls_used >= ball_limit){
                stream.log("Reached the limit of " + std::to_string(ball_limit) + " balls.", COLOR_RED);
                return {CatchResult::BALL_LIMIT_REACHED, balls_used};
            }
            continue;
        case 1:
            stream.log("BasicCatcher: End of battle detected.", COLOR_PURPLE);
            // It's actually fainted or caught. The logic to find out which one
            // is in basic_catcher().
            return {CatchResult::POKEMON_FAINTED, balls_used};
        case 2:
            return {CatchResult::OWN_FAINTED, balls_used};
        default:
            return {CatchResult::TIMED_OUT, balls_used};
        }
    }
}


CatchResults basic_catcher(
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const std::string& ball_slug, uint16_t ball_limit
){
    context.wait_for_all_requests();
    stream.log("Attempting to catch with: " + ball_slug);

    CatchResults results = throw_balls(stream, context, language, ball_slug, ball_limit);
    const std::string s = (results.balls_used <= 1 ? "" : "s");
    const std::string pokeball_str = std::to_string(results.balls_used) + " " + ball_slug + s;

    switch (results.result){
    case CatchResult::OUT_OF_BALLS:
        stream.log("BasicCatcher: Out of balls after throwing " + pokeball_str, COLOR_RED);
        return results;
    case CatchResult::CANNOT_THROW_BALL:
        stream.log("BasicCatcher: Unable to throw a ball.", COLOR_RED);
        return results;
    case CatchResult::BALL_LIMIT_REACHED:
        stream.log("BasicCatcher: Ball limit reached.", COLOR_RED);
        return results;
    case CatchResult::OWN_FAINTED:
        stream.log("BasicCatcher: Wwn " + Pokemon::STRING_POKEMON + " fainted after throwing " + pokeball_str, COLOR_RED);
        return results;
    case CatchResult::TIMED_OUT:
        stream.log("BasicCatcher: Timed out.", COLOR_RED);
        return results;
    default:;
    }

    //  Need to distinguish between caught or faint.
    //  Where there is no pokemon evolving, the order of events in BDSP is:
    //  exp screen -> lvl up and learn new move dialog -> new pokemon received screen if caught
    //  -> black screen -> return to overworld
    //  Wthere there is pokemon evolving, the order becomes:
    //  exp screen -> lvl up and learn new move dialog -> black screen -> pokemon evolving
    //  -> new pokemon received screen if caught.
    //  In this basic_catcher() we don't handle pokemon evolving.

    //  First, default the result to be fainted.
    results.result = CatchResult::POKEMON_FAINTED;
    size_t num_learned_moves = 0;
    while (true){
        context.wait_for_all_requests();
        //  Wait for end of battle.
        // BlackScreenOverWatcher black_screen_detector;
        EndBattleWatcher end_battle;
        //  Look for a pokemon learning a new move.
        SelectionArrowFinder learn_move(stream.overlay(), {0.50, 0.62, 0.40, 0.18}, COLOR_YELLOW);
        //  Look for the pokemon caught screen.
        ReceivePokemonDetector caught_detector;
        int ret = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
            },
            {
                {end_battle},
                {caught_detector},
                {learn_move},
            }
        );
        switch (ret){
        case 0:
            if (results.result == CatchResult::POKEMON_FAINTED){
                stream.log(
                    "BasicCatcher: The wild " + STRING_POKEMON + " fainted after " +
                    pokeball_str, COLOR_RED
                );
            }
            stream.log("BasicCatcher: Battle finished!", COLOR_BLUE);
            pbf_wait(context, TICKS_PER_SECOND);
            context.wait_for_all_requests();
            return results;
        case 1:
            if (results.result == CatchResult::POKEMON_CAUGHT){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "BasicCatcher: Found receive pokemon screen two times.",
                    stream
                );
            }
            stream.log("BasicCatcher: The wild " + STRING_POKEMON + " was caught by " + pokeball_str, COLOR_BLUE);
            pbf_wait(context, 50);
            results.result = CatchResult::POKEMON_CAUGHT;
            break; //  Continue the loop.
        case 2:
            stream.log("BasicCatcher: Detected move learn! Don't learn the new move.", COLOR_BLUE);
            num_learned_moves++;
            if (num_learned_moves == 100){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "BasicCatcher: Learn new move attempts reach 100.",
                    stream
                );
            }
            pbf_move_right_joystick(context, 128, 255, 20, 105);
            pbf_press_button(context, BUTTON_ZL, 20, 105);
            break; //  Continue the loop.

        default:
            stream.log("BasicCatcher: Timed out.", COLOR_RED);
            results.result = CatchResult::TIMED_OUT;
            return results;
        }
    }
}


}
}
}
