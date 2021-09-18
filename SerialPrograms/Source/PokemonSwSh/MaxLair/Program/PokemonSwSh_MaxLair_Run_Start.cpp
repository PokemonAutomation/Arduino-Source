/*  Max Lair Run Start
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/SwitchRoutines/SwitchDigitEntry.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h"
#include "PokemonSwSh_MaxLair_Run_Start.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void start_raid(
    MultiSwitchProgramEnvironment& env,
    GlobalStateTracker& state_tracker,
    QImage entrance[4],
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
        console.botbase().wait_for_all_requests();

        entrance[index] = console.video().snapshot();

        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        pbf_press_button(console, BUTTON_A, 10, 2 * TICKS_PER_SECOND);
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);

        if (is_host && boss_slot > 0){
            for (size_t c = 1; c < boss_slot; c++){
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
        if (console.index() != host.index()){
            pbf_wait(console, 3 * TICKS_PER_SECOND);
        }
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();

        LobbyDoneConnecting connecting_detector;
        wait_until(
            env, console,
            std::chrono::seconds(180),
            { &connecting_detector }
        );

        //  Ready up.
        pbf_press_button(console, BUTTON_A, 10, TICKS_PER_SECOND);
        console.botbase().wait_for_all_requests();

        LobbyDoneConnecting ready_detector;
        wait_until(
            env, console,
            std::chrono::seconds(180),
            { &connecting_detector }
        );

        //  Start raid.
        pbf_mash_button(host, BUTTON_A, 2 * TICKS_PER_SECOND);
    });

//    //  Start raid.
//    pbf_mash_button(host, BUTTON_A, 5 * TICKS_PER_SECOND);
}


}
}
}
}
