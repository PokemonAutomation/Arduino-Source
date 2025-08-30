/*  Max Lair State Machine
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/FrozenImageDetector.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ItemSelectMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ProfessorSwap.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Entrance.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_PokemonSelect.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_PathSelect.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_ItemSelect.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_ProfessorSwap.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Battle.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_PokemonSwap.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_CaughtScreen.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Entrance.h"
#include "PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



AdventureRuntime::~AdventureRuntime() = default;
AdventureRuntime::AdventureRuntime(
    FixedLimitVector<ConsoleHandle>& consoles,
    const size_t p_host_index,
    const Consoles& p_console_settings,
    const EndBattleDecider& p_actions,
    const bool p_go_home_when_done,
    HostingSettings& p_hosting_settings,
    EventNotificationOption& p_notification_status,
    EventNotificationOption& p_notification_shiny,
    Stats& p_session_stats
)
    : host_index(p_host_index)
    , console_settings(p_console_settings)
    , actions(p_actions)
    , go_home_when_done(p_go_home_when_done)
    , hosting_settings(p_hosting_settings)
    , notification_status(p_notification_status)
    , notification_shiny(p_notification_shiny)
    , ocr_watchdog(p_console_settings.active_consoles())
    , session_stats(p_session_stats)
{
    for (size_t c = 0; c < p_console_settings.active_consoles(); c++){
        ocr_watchdog.emplace_back(consoles[c].logger(), console_settings.PLAYERS[c]->language);
    }
}


StateMachineAction run_state_iteration(
    AdventureRuntime& runtime, size_t console_index,
    ProgramEnvironment& env,
    ConsoleHandle& console, ProControllerContext& context,
    bool save_path,
    GlobalStateTracker& global_state,
    const EndBattleDecider& decider,
    const ImageViewRGB32& entrance
){
    GlobalState state = global_state.infer_actual_state(console_index);
//    uint8_t wins = state.wins;
    bool starting = true;
    for (size_t c = 0; c < 4; c++){
        if (state.players[c].console_id == (int8_t)console_index){
            starting = false;
            break;
        }
    }

    PokemonSelectMenuDetector pokemon_select(false);
    PokemonSwapMenuDetector pokemon_swap(false);
    PathSelectDetector path_select;
    ItemSelectDetector item_menu(false);
    ProfessorSwapDetector professor_swap(console.overlay(), !starting);
    BattleMenuDetector battle_menu;
    RaidCatchDetector catch_select(console.overlay());
    PokemonCaughtMenuDetector caught_menu;
    EntranceDetector entrance_detector(entrance);
    FrozenImageDetector frozen_screen(COLOR_CYAN, {0, 0, 1, 0.5}, std::chrono::seconds(30), 10);

    int result = wait_until(
        console, context,
        std::chrono::seconds(300),
        {
            {starting
                ? (VisualInferenceCallback&)pokemon_select
                : (VisualInferenceCallback&)pokemon_swap
            },
            {path_select},
            {item_menu},
            {professor_swap},
            {battle_menu},
            {catch_select},
            {caught_menu},
            {entrance_detector},
            {frozen_screen},
        },
        std::chrono::milliseconds(200)
    );

    switch (result){
    case 0:
        if (starting){
            console.log("Current State: " + STRING_POKEMON + " Select");
            run_select_pokemon(
                console_index,
                console, context,
                global_state,
                runtime.ocr_watchdog[console_index],
                runtime.console_settings[console_index]
            );
            return StateMachineAction::KEEP_GOING;
        }else{
            console.log("Current State: " + STRING_POKEMON + " Swap");
            run_swap_pokemon(
                console_index,
                runtime,
                console, context,
                global_state,
                runtime.console_settings[console_index]
            );
            return StateMachineAction::KEEP_GOING;
        }
    case 1:
        console.log("Current State: Path Select");
        run_path_select(env, console_index, console, context, global_state);
        return StateMachineAction::KEEP_GOING;
    case 2:
        console.log("Current State: Item Select");
        run_item_select(console_index, console, context, global_state);
        return StateMachineAction::KEEP_GOING;
    case 3:
        console.log("Current State: Professor Swap");
        run_professor_swap(console_index, runtime, console, context, global_state);
        return StateMachineAction::KEEP_GOING;
    case 4:
        console.log("Current State: Move Select");
        return run_move_select(
            env, console_index,
            console, context,
            runtime.ocr_watchdog[console_index],
            global_state,
            runtime.console_settings[console_index],
            battle_menu.dmaxed(),
            battle_menu.cheer()
        );
    case 5:
        console.log("Current State: Catch Select");
        return throw_balls(
            runtime,
            env, console_index,
            console, context,
            runtime.console_settings[console_index].language,
            runtime.ocr_watchdog[console_index],
            global_state,
            decider
        );
    case 6:
        console.log("Current State: Caught Menu");
        return run_caught_screen(
            runtime,
            env, console_index,
            console, context,
            global_state, decider,
            entrance
        );
    case 7:
        console.log("Current State: Entrance");
        run_entrance(
            runtime,
            env, console_index,
            console, context,
            save_path,
            global_state
        );
        return StateMachineAction::DONE_WITH_ADVENTURE;
    case 8:
        console.log("Current State: Frozen Screen", COLOR_RED);
//        pbf_mash_button(context, BUTTON_B, TICKS_PER_SECOND);
//        context.wait_for_all_requests();
//        return StateMachineAction::KEEP_GOING;
        return StateMachineAction::RESET_RECOVER;
    default:
        console.log("Program hang. No state detected after 5 minutes.", COLOR_RED);
        dump_image(console.logger(), MODULE_NAME, console.video(), "ProgramHang");
        global_state.mark_as_dead(console_index);
        return StateMachineAction::RESET_RECOVER;
    }


}




}
}
}
}
