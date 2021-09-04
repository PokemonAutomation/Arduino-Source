/*  Max Lair Run Start
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/SwitchRoutines/SwitchDigitEntry.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h"
#include "PokemonSwSh_MaxLair_Run_Start.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void start_raid(
    MultiSwitchProgramEnvironment& env,
    GlobalStateTracker& state_tracker,
    ConsoleHandle& host, size_t boss_slot,
    const uint8_t code[8]
){
    env.log("Starting raid...");

    env.run_in_parallel([&](ConsoleHandle& console){
        size_t index = console.index();
        GlobalState& state = state_tracker[index];


        bool is_host = index == host.index();

        //  Clear quit prompts.
        pbf_press_button(console, BUTTON_A, 10, 10);
        pbf_mash_button(console, BUTTON_B, 5 * TICKS_PER_SECOND);

        //  Enter lobby.
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        pbf_press_button(console, BUTTON_A, 10, 2 * TICKS_PER_SECOND);
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);

        if (is_host){
            for (size_t c = 0; c < boss_slot; c++){
                pbf_press_dpad(console, DPAD_DOWN, 10, 50);
            }
            pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        }else{
            pbf_press_button(console, BUTTON_B, 10, TICKS_PER_SECOND);
        }
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        pbf_press_button(console, BUTTON_A, 10, 5 * TICKS_PER_SECOND);

        //  Read boss.
        if (is_host){
            console.botbase().wait_for_all_requests();
            DenMonReader reader(host, host);
            QImage screen = host.video().snapshot();
            DenMonReadResults results = reader.read(screen);
            auto iter = results.slugs.slugs.begin();
            if (iter != results.slugs.slugs.end() && iter->first < 50){
                state.boss = results.slugs.slugs.begin()->second;
            }
        }

        //  Prepare
        if (env.consoles.size() == 1){
            pbf_press_dpad(console, DPAD_DOWN, 10, 50);
        }else{
            pbf_press_button(console, BUTTON_PLUS, 10, TICKS_PER_SECOND);
            enter_digits(console, 8, code);
            pbf_wait(console, 2 * TICKS_PER_SECOND);
            pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        }
    });

    if (env.consoles.size() == 1){
        //  Start raid.
        pbf_press_button(host, BUTTON_A, 10, TICKS_PER_SECOND);
        host.botbase().wait_for_all_requests();
        return;
    }

    //  Join raid and ready up.
    env.run_in_parallel([&](ConsoleHandle& console){
        InterruptableCommandSession commands(console);

        if (console.index() == host.index()){
            pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
            console.botbase().wait_for_all_requests();

            LobbyAllReadyDetector ready_detector(console, env.consoles.size());
            ready_detector.register_command_stop(commands);

            AsyncVisualInferenceSession inference(env, console);
            inference += ready_detector;

            commands.run([&](const BotBaseContext& context){
                pbf_wait(context, 180 * TICKS_PER_SECOND);
                context->wait_for_all_requests();
            });


        }else{
            pbf_wait(console, 3 * TICKS_PER_SECOND);
            pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
            console.botbase().wait_for_all_requests();

            LobbyDoneConnecting connecting_detector(console);
            connecting_detector.register_command_stop(commands);

            AsyncVisualInferenceSession inference(env, console);
            inference += connecting_detector;

            commands.run([&](const BotBaseContext& context){
                pbf_wait(context, 180 * TICKS_PER_SECOND);
                context->wait_for_all_requests();
            });

            pbf_wait(console, TICKS_PER_SECOND);
            pbf_mash_button(console, BUTTON_A, TICKS_PER_SECOND);
        }
    });

    //  Start raid.
    pbf_mash_button(host, BUTTON_A, 5 * TICKS_PER_SECOND);
}


}
}
}
}
