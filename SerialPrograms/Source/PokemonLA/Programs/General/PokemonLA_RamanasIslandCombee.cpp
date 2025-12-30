/*  Ramanas Island Combee Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/PokemonLA_BlackOutDetector.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/General/PokemonLA_RamanasIslandCombee.h"
#include "PokemonLA/Programs/PokemonLA_LeapPokemonActions.h"
#include "CommonFramework/GlobalSettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;



RamanasCombeeFinder_Descriptor::RamanasCombeeFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:Ramanas Island Combee Finder",
        STRING_POKEMON + " LA", "Ramanas Combee Finder",
        "Programs/PokemonLA/RamanasCombeeFinder.html",
        "Check Ramanas Island Tree until a Combee is found.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class RamanasCombeeFinder_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , trees(m_stats["Trees"])
        , errors(m_stats["Errors"])
        , blackouts(m_stats["Blackouts"])
        , found(m_stats["Found"])
        , enroute_shinies(m_stats["Enroute Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Trees");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Blackouts", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Found");
        m_display_order.emplace_back("Enroute Shinies");
        m_aliases["Shinies"] = "Enroute Shinies";
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& trees;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& blackouts;
    std::atomic<uint64_t>& found;
    std::atomic<uint64_t>& enroute_shinies;
};
std::unique_ptr<StatsTracker> RamanasCombeeFinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


RamanasCombeeFinder:: RamanasCombeeFinder()
    : LANGUAGE(
        "<b>Game Language</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , SHINY_DETECTED_ENROUTE(
        "Enroute Shiny Action",
        "This applies if a shiny is detected while traveling in the overworld.",
        "0 ms"
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED_ENROUTE.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
    , SAVE_DEBUG_VIDEO(
        "<b>Save debug videos to Switch:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(SHINY_DETECTED_ENROUTE);
    PA_ADD_OPTION(NOTIFICATIONS);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(SAVE_DEBUG_VIDEO);
    }
}


void RamanasCombeeFinder::check_tree_no_stop(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    disable_shiny_sound(context);
    // Throw pokemon
    pbf_press_button(context, BUTTON_ZR, 500ms, 1500ms);
    context.wait_for_all_requests();
    env.current_stats<RamanasCombeeFinder_Descriptor::Stats>().trees++;
    env.update_stats();
}

bool RamanasCombeeFinder::check_tree(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();

    disable_shiny_sound(context);
    bool battle_found = check_tree_or_ore_for_battle(env.console, context);
    env.current_stats<RamanasCombeeFinder_Descriptor::Stats>().trees++;
    env.update_stats();

    context.wait_for_all_requests();

    bool ret = false;
    if (battle_found){
        ret = handle_battle(env, context);
    }

    enable_shiny_sound(context);

    return ret;
}


void RamanasCombeeFinder::disable_shiny_sound(ProControllerContext& context){
    context.wait_for_all_requests();
    m_enable_shiny_sound.store(false, std::memory_order_release);
}
void RamanasCombeeFinder::enable_shiny_sound(ProControllerContext& context){
    context.wait_for_all_requests();
    m_enable_shiny_sound.store(true, std::memory_order_release);
}

bool RamanasCombeeFinder::handle_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    RamanasCombeeFinder_Descriptor::Stats& stats = env.current_stats<RamanasCombeeFinder_Descriptor::Stats>();

    PokemonDetails pokemon = get_pokemon_details(env.console, context, LANGUAGE);

    pbf_press_button(context, BUTTON_B, 160ms, 1800ms);

    context.wait_for_all_requests();

    if (pokemon.name_candidates.find("combee") == pokemon.name_candidates.end()){
        env.console.log("Not a Combee, leaving battle.");
        exit_battle(env.console, context, ExitBattleMethod::RunAway);
        return false;
    }

    env.console.log("Combee found");
    stats.found++;

    context.wait_for_all_requests();
    throw ProgramFinishedException();
//    on_battle_match_found(env, env.console, context, SHINY_DETECTED_ENROUTE, true);
//    return true;
}

void RamanasCombeeFinder::grouped_path(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    BattleMenuDetector battle_menu_detector(env.console, env.console, true);

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){

            env.console.log("Checking Tree 1");
            change_mount(env.console,context,MountState::BRAVIARY_ON);
            pbf_move_left_joystick(context, {+0.874, +1}, 800ms, 160ms);
            pbf_press_button(context, BUTTON_B, 19120ms, 0ms);
            pbf_press_button(context, BUTTON_Y, 3040ms, 0ms);
            pbf_move_right_joystick(context, {0, -1}, 720ms, 160ms);
            check_tree_no_stop(env, context);

            env.console.log("Checking Tree 2");
            pbf_press_button(context, BUTTON_PLUS, 160ms, 1600ms);
            pbf_move_left_joystick(context, {+0.898, +1}, 800ms, 160ms);
            context.wait_for_all_requests();
            enable_shiny_sound(context);
            pbf_press_button(context, BUTTON_B, 3360ms, 0ms);
            pbf_press_button(context, BUTTON_Y, 3040ms, 0ms);
            pbf_move_right_joystick(context, {0, -1}, 720ms, 160ms);
            check_tree_no_stop(env, context);

            env.console.log("Checking Tree 3");
            pbf_press_button(context, BUTTON_PLUS, 160ms, 1600ms);
            pbf_move_left_joystick(context, {-1, +0.531}, 800ms, 160ms);
            context.wait_for_all_requests();
            enable_shiny_sound(context);
            pbf_press_button(context, BUTTON_B, 2800ms, 0ms);
            pbf_press_button(context, BUTTON_Y, 3040ms, 0ms);
            pbf_move_right_joystick(context, {0, -1}, 720ms, 160ms);
            check_tree_no_stop(env, context);

            env.console.log("Checking Tree 4");
            pbf_press_button(context, BUTTON_PLUS, 160ms, 1600ms);
            pbf_move_left_joystick(context, {-0.609, -1}, 800ms, 160ms);
            context.wait_for_all_requests();
            enable_shiny_sound(context);
            pbf_press_button(context, BUTTON_B, 3000ms, 0ms);
            pbf_press_button(context, BUTTON_Y, 3040ms, 0ms);
            pbf_move_right_joystick(context, {0, -1}, 720ms, 160ms);
            check_tree_no_stop(env, context);

            env.console.log("Checking Tree 5");
            pbf_press_button(context, BUTTON_PLUS, 160ms, 1600ms);
            pbf_move_left_joystick(context, {+0.567, +1}, 800ms, 160ms);
            context.wait_for_all_requests();
            enable_shiny_sound(context);
            pbf_press_button(context, BUTTON_B, 680ms, 0ms);
            pbf_press_button(context, BUTTON_Y, 3040ms, 0ms);
            pbf_move_right_joystick(context, {0, -1}, 720ms, 160ms);
        },
        {
            {battle_menu_detector},
        }
    );

    if (ret == 0){
        if (handle_battle(env, context)){
            env.console.log("Battle found before last tree in the path.");
        }

    }else{
        check_tree(env, context);
        env.console.log("Checked all trees in the path.");
    }

}

void RamanasCombeeFinder::run_iteration(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    bool fresh_from_reset
){
    RamanasCombeeFinder_Descriptor::Stats& stats = env.current_stats<RamanasCombeeFinder_Descriptor::Stats>();
    stats.attempts++;
    env.update_stats();
    env.console.log("Starting route and shiny detection...");

    for (size_t c = 0; true; c++){
        context.wait_for_all_requests();
        if (is_pokemon_selection(env.console, env.console.video().snapshot())){
            break;
        }
        if (c >= 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to switch to Pokemon selection after 5 attempts.",
                env.console
            );
        }
        env.console.log("Not on Pokemon selection. Attempting to switch to it...", COLOR_ORANGE);
        pbf_press_button(context, BUTTON_X, 160ms, 1840ms);
    }

    float shiny_coefficient = 1.0;

    ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
        //  Warning: This callback will be run from a different thread than this function.
        if (!m_enable_shiny_sound.load()){
            return false;
        }
        stats.enroute_shinies++;
        shiny_coefficient = error_coefficient;
        return on_shiny_callback(env, env.console, SHINY_DETECTED_ENROUTE, error_coefficient);
    });

    BlackOutDetector black_out_detector(env.console, env.console);

    goto_camp_from_jubilife(
        env, env.console, context,
        TravelLocations::instance().Fieldlands_Heights,
        fresh_from_reset
    );

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            grouped_path(env, context);
            context.wait_for_all_requests();
            goto_camp_from_overworld(env, env.console, context);
            goto_professor(env.console, context, Camp::FIELDLANDS_FIELDLANDS);
        },
        {{shiny_detector, black_out_detector}}
    );
    shiny_detector.throw_if_no_sound();
    if (ret == 0){
        on_shiny_sound(env, env.console, context, SHINY_DETECTED_ENROUTE, shiny_coefficient);
    }else if (ret == 1){
        env.log("Character blacks out");
        // black out.
        stats.blackouts++;
        env.update_stats();
        if (SAVE_DEBUG_VIDEO){
            // Take a video to know why it blacks out
            pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 2000ms);
            context.wait_for_all_requests();
        }
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Black out.",
            env.console
        );
    }

    from_professor_return_to_jubilife(env, env.console, context);
}

void RamanasCombeeFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    RamanasCombeeFinder_Descriptor::Stats& stats = env.current_stats<RamanasCombeeFinder_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 40ms, 40ms);

    bool fresh_from_reset = false;
    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);
        try{
            run_iteration(env, context, fresh_from_reset);
        }catch (OperationFailedException& e){
            stats.errors++;
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
            fresh_from_reset = reset_game_from_home(env, env.console, context);
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}
