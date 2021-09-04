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
#include "CommonFramework/Inference/VisualInferenceSession.h"
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
    for (size_t c = 0; c < 30; c++){
        pbf_press_dpad(console, DPAD_RIGHT, 10, 50);
        console.botbase().wait_for_all_requests();
        frame = console.video().snapshot();
        std::string current_ball = reader.read_ball(frame);
        if (current_ball == ball_slug){
            return true;
        }
        if (current_ball == first_ball){
            return false;
        }
    }
    return false;
}


CatchResults basic_catcher(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    Language language,
    const std::string& ball_slug
){
    console.botbase().wait_for_all_requests();
    env.log("Attempting to catch with: " + ball_slug);

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

        InterruptableCommandSession commands(console);

        StandardBattleMenuDetector menu_detector(console, false);
        menu_detector.register_command_stop(commands);

        ExperienceGainDetector experience_detector(console);
        experience_detector.register_command_stop(commands);

        AsyncVisualInferenceSession inference(env, console);
        inference += menu_detector;
        inference += experience_detector;

        commands.run([](const BotBaseContext& context){
            pbf_wait(context, 60 * TICKS_PER_SECOND);
            context->wait_for_all_requests();
        });

        if (menu_detector.triggered()){
            env.log("Failed to catch.", "orange");
            continue;
        }

        //  Experience screen.
        if (experience_detector.triggered()){
            env.log("End of battle detected.", "purple");
            break;
        }

//        PA_THROW_StringException("Program is stuck. Did your " + STRING_POKEMON + " faint?");
        return {CatchResult::TIMEOUT, balls_used};
    }

    //  Wait for end of battle.
    {
        InterruptableCommandSession commands(console);

        BlackScreenDetector black_screen_detector(console);
        black_screen_detector.register_command_stop(commands);

        AsyncVisualInferenceSession inference(env, console);
        inference += black_screen_detector;

        commands.run([=](const BotBaseContext& context){
            pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
            context.botbase().wait_for_all_requests();
        });
    }

    //  Look for the orange caught screen.
    {
        InterruptableCommandSession commands(console);

        ReceivePokemonDetector caught_detector(console);
        caught_detector.register_command_stop(commands);

        AsyncVisualInferenceSession inference(env, console);
        inference += caught_detector;

        commands.run([](const BotBaseContext& context){
            pbf_mash_button(context, BUTTON_B, 4 * TICKS_PER_SECOND);
            context->wait_for_all_requests();
        });

        if (caught_detector.triggered()){
            env.log("The wild " + STRING_POKEMON + " was caught!", "blue");
        }else{
            env.log("The wild " + STRING_POKEMON + " fainted.", "red");
            return {CatchResult::POKEMON_FAINTED, balls_used};
        }
    }

//    pbf_wait(console, 5 * TICKS_PER_SECOND);

#if 1
    //  Wait to return to overworld.
    {
        InterruptableCommandSession commands(console);

        BlackScreenDetector black_screen_detector(console);
        black_screen_detector.register_command_stop(commands);

        AsyncVisualInferenceSession inference(env, console);
        inference += black_screen_detector;

        commands.run([=](const BotBaseContext& context){
            pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
            context.botbase().wait_for_all_requests();
        });
    }
#endif

    return {CatchResult::POKEMON_CAUGHT, balls_used};
}


}
}
}
