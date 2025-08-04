/*  Basic Pokemon Catcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/PokemonSwSh_ReceivePokemonDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_ExperienceGainDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh_BasicCatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


//  Returns the # of slots scrolled. Returns -1 if not found.
int move_to_ball(
    const BattleBallReader& reader,
    VideoStream& stream, ProControllerContext& context,
    const std::string& ball_slug,
    bool forward, int attempts, Milliseconds delay
){
    VideoSnapshot frame = stream.video().snapshot();
    std::string first_ball = reader.read_ball(frame);
    if (first_ball == ball_slug){
        return 0;
    }

    size_t repeat_counter = 0;
    for (int c = 1; c < attempts; c++){
        pbf_press_dpad(context, forward ? DPAD_RIGHT : DPAD_LEFT, 80ms, delay);
        context.wait_for_all_requests();
        frame = stream.video().snapshot();
        std::string current_ball = reader.read_ball(frame);
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
    const BattleBallReader& reader, VideoStream& stream, ProControllerContext& context,
    const std::string& ball_slug
){
    //  Search forward at high speed.
    int ret = move_to_ball(reader, stream, context, ball_slug, true, 100, 320ms);
    if (ret < 0){
        return 0;
    }
    if (ret == 0){
        uint16_t quantity = reader.read_quantity(stream.video().snapshot());
        return quantity == 0 ? -1 : quantity;
    }

    //  Wait a second to let the video catch up.
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();

    //  Now try again in reverse at a lower speed in case we overshot.
    //  This will return immediately if we got it right the first time.
    ret = move_to_ball(reader, stream, context, ball_slug, false, 5, 1000ms);
    if (ret < 0){
        return 0;
    }
    if (ret > 0){
        stream.log("Fast ball scrolling overshot by " + std::to_string(ret) + " slot(s).", COLOR_RED);
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
        {
            BattleBallReader reader(stream, language);

            pbf_mash_button(context, BUTTON_X, 125);
            context.wait_for_all_requests();

            bool success = move_to_ball(reader, stream, context, ball_slug);
            if (!success){
                return {CatchResult::OUT_OF_BALLS, balls_used};
            }

            pbf_mash_button(context, BUTTON_A, 125);
            context.wait_for_all_requests();
        }
        balls_used++;

        auto start = current_time();

        StandardBattleMenuWatcher menu_detector(false);
        ExperienceGainWatcher experience_detector;
        int result = wait_until(
            stream, context,
            std::chrono::seconds(60),
            {
                {menu_detector},
                {experience_detector},
            }
        );
        switch (result){
        case 0:
            if (current_time() < start + std::chrono::seconds(5)){
                stream.log("BasicCatcher: Unable to throw ball.", COLOR_RED);
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
            return {CatchResult::POKEMON_FAINTED, balls_used};
        default:
            stream.log("BasicCatcher: Timed out.", COLOR_RED);
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
    switch (results.result){
    case CatchResult::OUT_OF_BALLS:
    case CatchResult::CANNOT_THROW_BALL:
    case CatchResult::BALL_LIMIT_REACHED:
    case CatchResult::TIMED_OUT:
        return results;
    default:;
    }


    //  Need to distinguish between caught or faint.


    //  Wait for end of battle.
//    console.video().snapshot()->save("test0.png");
    {
        stream.log("Waiting for black screen end...");
        BlackScreenOverWatcher black_screen_detector;
        run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
            },
            {{black_screen_detector}}
        );
        stream.log("Waiting for black screen end... Found!");
    }

    //  Look for the orange caught screen.
//    console.video().snapshot()->save("test1.png");
//    context.wait_for(std::chrono::milliseconds(5000));
    {
        ReceivePokemonOverWatcher caught_detector(true);

        int result = run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 4 * TICKS_PER_SECOND);
            },
            {{caught_detector}}
        );

        switch (result){
        case 0:
            stream.log("BasicCatcher: The wild " + STRING_POKEMON + " was caught!", COLOR_BLUE);
            break;
        default:
            stream.log("BasicCatcher: The wild " + STRING_POKEMON + " fainted.", COLOR_RED);
            results.result = CatchResult::POKEMON_FAINTED;
            return results;
        }
    }

//    pbf_wait(context, 5 * TICKS_PER_SECOND);
//    console.video().snapshot()->save("test2.png");
    {
        stream.log("Waiting for black screen end...");
        BlackScreenOverWatcher black_screen_detector;
        run_until<ProControllerContext>(
            stream, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
            },
            {{black_screen_detector}}
        );
        stream.log("Waiting for black screen end... Found!");
    }

    results.result = CatchResult::POKEMON_CAUGHT;
    return results;
}


}
}
}
