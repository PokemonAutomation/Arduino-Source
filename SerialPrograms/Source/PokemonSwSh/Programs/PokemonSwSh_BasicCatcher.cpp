/*  Basic Pokemon Catcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

//#include <QtGlobal>
//#include "Common/Cpp/Exception.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_ReceivePokemonDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_ExperienceGainDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh_BasicCatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


//  Returns true if the ball is found.
bool move_to_ball(
    const BattleBallReader& reader,
    ConsoleHandle& console,
    const std::string& ball_slug
){
    QImage frame = console.video().snapshot();
    std::string first_ball = reader.read_ball(frame);
    if (first_ball == ball_slug){
        return true;
    }
    size_t repeat_counter = 0;
    for (size_t c = 0; c < 50; c++){
        pbf_press_dpad(console, DPAD_RIGHT, 10, 50);
        console.botbase().wait_for_all_requests();
        frame = console.video().snapshot();
        std::string current_ball = reader.read_ball(frame);
        if (current_ball == ball_slug){
            return true;
        }
        if (current_ball == first_ball){
            repeat_counter++;
            if (repeat_counter == 3){
                return false;
            }
        }
    }
    return false;
}


CatchResults throw_balls(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    Language language,
    const std::string& ball_slug
){
    uint16_t balls_used = 0;
    while (true){
        {
            BattleBallReader reader(console, language);

            pbf_mash_button(console, BUTTON_X, 125);
            console.botbase().wait_for_all_requests();

            bool success = move_to_ball(reader, console, ball_slug);
            if (!success){
//                PA_THROW_StringException("Unable to find appropriate ball.");
                return {CatchResult::OUT_OF_BALLS, balls_used};
            }

            pbf_mash_button(console, BUTTON_A, 125);
            console.botbase().wait_for_all_requests();
        }
        balls_used++;

        auto start = std::chrono::system_clock::now();

        StandardBattleMenuDetector menu_detector(false);
        ExperienceGainDetector experience_detector(console);
        int result = wait_until(
            env, console,
            std::chrono::seconds(60),
            {
                &menu_detector,
                &experience_detector,
            }
        );
        switch (result){
        case 0:
            if (std::chrono::system_clock::now() < start + std::chrono::seconds(5)){
                env.log("BasicCatcher: Unable to throw ball.", Qt::red);
                return {CatchResult::CANNOT_THROW_BALL, balls_used};
            }
            env.log("BasicCatcher: Failed to catch.", "orange");
            continue;
        case 1:
            env.log("BasicCatcher: End of battle detected.", "purple");
            return {CatchResult::POKEMON_FAINTED, balls_used};
         default:
            env.log("BasicCatcher: Timed out.", Qt::red);
            return {CatchResult::TIMEOUT, balls_used};
        }
    }
}


CatchResults basic_catcher(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    Language language,
    const std::string& ball_slug
){
    console.botbase().wait_for_all_requests();
    env.log("Attempting to catch with: " + ball_slug);

    CatchResults results = throw_balls(env, console, language, ball_slug);
    if (results.result == CatchResult::OUT_OF_BALLS){
        return results;
    }
    if (results.result == CatchResult::CANNOT_THROW_BALL){
        return results;
    }
    if (results.result == CatchResult::TIMEOUT){
        return results;
    }


    //  Need to distinguish between caught or faint.


    //  Wait for end of battle.
    {
        BlackScreenDetector black_screen_detector;
        run_until(
            env, console,
            [=](const BotBaseContext& context){
                pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
                context.botbase().wait_for_all_requests();
            },
            { &black_screen_detector }
        );
    }

    //  Look for the orange caught screen.
    {
        ReceivePokemonDetector caught_detector;

        int result = run_until(
            env, console,
            [=](const BotBaseContext& context){
                pbf_mash_button(context, BUTTON_B, 4 * TICKS_PER_SECOND);
                context->wait_for_all_requests();
            },
            { &caught_detector }
        );

        switch (result){
        case 0:
            env.log("BasicCatcher: The wild " + STRING_POKEMON + " was caught!", "blue");
            break;
        default:
            env.log("BasicCatcher: The wild " + STRING_POKEMON + " fainted.", "red");
            results.result = CatchResult::POKEMON_FAINTED;
            return results;
        }
    }

//    pbf_wait(console, 5 * TICKS_PER_SECOND);
    {
        BlackScreenDetector black_screen_detector;
        run_until(
            env, console,
            [=](const BotBaseContext& context){
                pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
                context.botbase().wait_for_all_requests();
            },
            { &black_screen_detector }
        );
    }

    results.result = CatchResult::POKEMON_CAUGHT;
    return results;
}


}
}
}
