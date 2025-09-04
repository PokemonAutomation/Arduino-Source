/*  Burmy Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Notification.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/PokemonLA_BlackOutDetector.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/ShinyHunting/PokemonLA_BurmyFinder.h"
#include "PokemonLA/Programs/PokemonLA_LeapPokemonActions.h"
#include "CommonFramework/GlobalSettingsPanel.h"

#ifdef _MSC_VER
#pragma warning(disable:4244)   //  double -> int precision loss
#endif

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;



BurmyFinder_Descriptor::BurmyFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:Burmy Hunter",
        STRING_POKEMON + " LA", "Burmy Hunter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/BurmyHunter.md",
        "Check nearby trees for a possible Shiny, Alpha or Alpha Shiny Burmy",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class BurmyFinder_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , trees(m_stats["Trees"])
        , errors(m_stats["Errors"])
        , blackouts(m_stats["Blackouts"])
        , found(m_stats["Found"])
        , enroute_shinies(m_stats["Enroute Shinies"])
        , tree_alphas(m_stats["Tree Alphas"])
        , tree_shinies(m_stats["Tree Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Trees");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Blackouts", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Found");
        m_display_order.emplace_back("Enroute Shinies");
        m_display_order.emplace_back("Tree Alphas");
        m_display_order.emplace_back("Tree Shinies");
        m_aliases["Shinies"] = "Enroute Shinies";
        m_aliases["Alphas"] = "Tree Alphas";
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& trees;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& blackouts;
    std::atomic<uint64_t>& found;
    std::atomic<uint64_t>& enroute_shinies;
    std::atomic<uint64_t>& tree_alphas;
    std::atomic<uint64_t>& tree_shinies;

};
std::unique_ptr<StatsTracker> BurmyFinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


BurmyFinder::BurmyFinder()
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
    , FOUND_SHINY_OR_ALPHA(
        "Found Shiny or Alpha",
        true, true,
        ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , MATCH_DETECTED_OPTIONS(
        "Match Action",
        "What to do when a Burmy is found that matches the \"Stop On\" parameter.",
        "Found Shiny or Alpha"
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED_ENROUTE.NOTIFICATIONS,
        &FOUND_SHINY_OR_ALPHA,
        &MATCH_DETECTED_OPTIONS.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
    , SAVE_DEBUG_VIDEO(
        "<b>Save debug videos to Switch:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(STOP_ON);
    PA_ADD_OPTION(EXIT_METHOD);
    PA_ADD_OPTION(SHINY_DETECTED_ENROUTE);
    PA_ADD_OPTION(MATCH_DETECTED_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(SAVE_DEBUG_VIDEO);
    }
}



struct BurmyFinder::TreeCounter{
    std::array<uint64_t, 16> tree = {};
    
    void log(Logger& logger) const{
        std::ostringstream oss;
        for(size_t i = 0; i < size(tree); i++){
            if (i != 0){
                oss << " - ";
            }
            oss << "Tree " << i << ": " << tostr_u_commas(tree[i]);
        }
        logger.log(oss.str());
    }
};

bool BurmyFinder::handle_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    BurmyFinder_Descriptor::Stats& stats = env.current_stats<BurmyFinder_Descriptor::Stats>();

    PokemonDetails pokemon = get_pokemon_details(env.console, context, LANGUAGE);
    pbf_press_button(context, BUTTON_B, 20, 225);
    context.wait_for_all_requests();

    if (pokemon.name_candidates.find("burmy") == pokemon.name_candidates.end()){
        env.console.log("Not a burmy. Leaving battle.");
        exit_battle(env.console, context, EXIT_METHOD);
        return false;
    }

    stats.found++;
    //  Match validation

    if (pokemon.is_alpha && pokemon.is_shiny){
        env.console.log("Found Shiny Alpha!");
        stats.tree_shinies++;
        stats.tree_alphas++;
    }else if (pokemon.is_alpha){
        env.console.log("Found Alpha!");
        stats.tree_alphas++;
    }else if (pokemon.is_shiny){
        env.console.log("Found Shiny!");
        stats.tree_shinies++;
    }else{
        env.console.log("Normie in the tree -_-");
    }
    env.update_stats();

    bool is_match = false;
    switch (STOP_ON){
    case StopOn::Shiny:
        is_match = pokemon.is_shiny;
        break;
    case StopOn::Alpha:
        is_match = pokemon.is_alpha;
        break;
    case StopOn::ShinyOrAlpha:
        is_match = pokemon.is_alpha || pokemon.is_shiny;
        break;
    case StopOn::ShinyAndAlpha:
        is_match = pokemon.is_alpha && pokemon.is_shiny;
        break;
    }

    bool notification_sent = false;
    do{
        std::string str;
        if (pokemon.is_shiny){
            if (pokemon.is_alpha){
                str = "Found Shiny Alpha!";
            }else{
                str = "Found Shiny!";
            }
        }else{
            if (pokemon.is_alpha){
                str = "Found Alpha!";
            }else{
                break;
            }
        }
        notification_sent |= send_program_notification(
            env, FOUND_SHINY_OR_ALPHA,
            Pokemon::COLOR_STAR_SHINY,
            std::move(str),
            {}, "",
            env.console.video().snapshot(), true
        );
    }while (false);

    if (is_match){
        on_battle_match_found(env, env.console, context, MATCH_DETECTED_OPTIONS, !notification_sent);
    }

    exit_battle(env.console, context, EXIT_METHOD);

    return true;
}


bool BurmyFinder::check_tree(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();

    disable_shiny_sound(context);
    bool battle_found = check_tree_or_ore_for_battle(env.console, context);
    env.current_stats<BurmyFinder_Descriptor::Stats>().trees++;
    env.update_stats();

    context.wait_for_all_requests();

    bool ret = false;
    if (battle_found){
        ret = handle_battle(env, context);
    }

    enable_shiny_sound(context);

    return ret;
}

void BurmyFinder::check_tree_no_stop(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    context.wait_for_all_requests();
    disable_shiny_sound(context);
    // Throw pokemon
    pbf_press_button(context, BUTTON_ZR, (0.5 * TICKS_PER_SECOND), 1.5 * TICKS_PER_SECOND);
    context.wait_for_all_requests();
    env.current_stats<BurmyFinder_Descriptor::Stats>().trees++;
    env.update_stats();
//    enable_shiny_sound(context);
}

void BurmyFinder::disable_shiny_sound(ProControllerContext& context){
    context.wait_for_all_requests();
    m_enable_shiny_sound.store(false, std::memory_order_release);
}
void BurmyFinder::enable_shiny_sound(ProControllerContext& context){
    context.wait_for_all_requests();
    m_enable_shiny_sound.store(true, std::memory_order_release);
}

void BurmyFinder::go_to_height_camp(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    const bool stop_on_detected = true;
    BattleMenuDetector battle_menu_detector(env.console, env.console, stop_on_detected);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
        },
        {
            {battle_menu_detector},
        }
    );
    if (ret == 0){
        env.log("Warning: found inside a battle when trying to return to camp.");
        // Unexpected battle during movement to camp
        if (SAVE_DEBUG_VIDEO){
            // Take a video to know why it enters battle during return trip
            pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
            context.wait_for_all_requests();
        }
        // Finish battle
        handle_battle(env, context);
        // Go back to camp
        goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
    }
}

size_t BurmyFinder::grouped_path(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t path, TreeCounter& tree_counter){

    size_t last_checked_tree = 0;

    if (path != 0){
        go_to_height_camp(env, context);
    }

    env.console.log("Currently Checking Path:" + std::to_string(path));

    BattleMenuDetector battle_menu_detector(env.console, env.console, true);

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            switch (path){
            case 0:
                //============ Tree 0=============//
                env.console.log("Checking tree: 0");
                pbf_move_left_joystick(context, 255, 85, 20, (0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_ZL, 20, (0.6 * TICKS_PER_SECOND));
                change_mount(env.console, context, MountState::BRAVIARY_ON);
                pbf_wait(context, 0.7 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_B, (5.1 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_Y, (4.2 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_right_joystick(context, 128, 255, (0.3 * TICKS_PER_SECOND), (0.2 * TICKS_PER_SECOND));
                last_checked_tree = 0;
                check_tree_no_stop(env, context);

                //============ Tree 1=============//
                env.console.log("Checking tree: 1");
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_left_joystick(context, 255, 10, 1 * TICKS_PER_SECOND, 0);
                context.wait_for_all_requests();
                enable_shiny_sound(context);
                pbf_press_button(context, BUTTON_B, (0.6 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_Y, (2.7 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_right_joystick(context, 128, 255, (0.1 * TICKS_PER_SECOND), (0.3 * TICKS_PER_SECOND));
                last_checked_tree = 1;
                check_tree_no_stop(env, context);

                //============ Tree 2=============//
                env.console.log("Checking tree: 2");
                pbf_press_button(context, BUTTON_PLUS, 20, 100);
                pbf_move_left_joystick(context, 255, 130, 1 * TICKS_PER_SECOND, 0);
                context.wait_for_all_requests();
                enable_shiny_sound(context);
                pbf_press_button(context, BUTTON_B, (0.6 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_Y, (2.4 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_right_joystick(context, 128, 255, (0.1 * TICKS_PER_SECOND), (0.3 * TICKS_PER_SECOND));
                last_checked_tree = 2;
                check_tree_no_stop(env, context);

                //============ Tree 3=============//
                env.console.log("Checking tree: 3");
                pbf_press_button(context, BUTTON_PLUS, 20, 100);
                pbf_move_left_joystick(context, 0, 95, 1 * TICKS_PER_SECOND, 0);
                context.wait_for_all_requests();
                enable_shiny_sound(context);
                pbf_press_button(context, BUTTON_B, 110, 0);
                pbf_press_button(context, BUTTON_Y, (2.3 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_right_joystick(context, 128, 255, (0.1 * TICKS_PER_SECOND), (0.3 * TICKS_PER_SECOND));
                last_checked_tree = 3;
                break;
            case 1:
                //============ Tree 4=============//
                env.console.log("Checking tree: 4");
                pbf_move_left_joystick(context, 255, 165, 20, (0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
                change_mount(env.console, context, MountState::BRAVIARY_ON);
                pbf_wait(context, 0.6 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_B, (4.8 * TICKS_PER_SECOND), (0.6 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_Y, (1.7 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_right_joystick(context, 127, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                last_checked_tree = 4;
                check_tree_no_stop(env, context);

                //============ Tree 5=============//
                env.console.log("Checking tree: 5");
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_left_joystick(context, 255, 190, 1 * TICKS_PER_SECOND, 0);
                context.wait_for_all_requests();
                enable_shiny_sound(context);
                pbf_press_button(context, BUTTON_B, (0.5 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_Y, (2.6 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_right_joystick(context, 127, 255, (0.15 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                last_checked_tree = 5;
                check_tree_no_stop(env, context);

                //============ Tree 6=============//
                env.console.log("Checking tree: 6");
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_left_joystick(context, 0, 130, 1 * TICKS_PER_SECOND, 0);
                context.wait_for_all_requests();
                enable_shiny_sound(context);
                pbf_press_button(context, BUTTON_B, (1.3 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_Y, (3.2 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_right_joystick(context, 127, 255, (0.15 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                last_checked_tree = 6;
                break;
            case 2:
                //============ Tree 7=============//
                env.console.log("Checking tree: 7");
                pbf_move_left_joystick(context, 180, 255, 20, (0.5 * TICKS_PER_SECOND));
                change_mount(env.console, context, MountState::BRAVIARY_ON);
                pbf_press_button(context, BUTTON_B, (7.2 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_Y, (4.2 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_left_joystick(context, 0, 127, (1.5 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_ZL, 20, 20);
                pbf_move_right_joystick(context, 127, 255, (0.5 * TICKS_PER_SECOND), 20);
                last_checked_tree = 7;
                check_tree_no_stop(env, context);

                //============ Tree 8=============//
                env.console.log("Checking tree: 8");
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_left_joystick(context, 255, 230, (0.5 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                context.wait_for_all_requests();
                enable_shiny_sound(context);
                pbf_press_button(context, BUTTON_Y, (1.7 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_right_joystick(context, 127, 255, (0.5 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                last_checked_tree = 8;
                check_tree_no_stop(env, context);

                //============ Tree 9=============//
                env.console.log("Checking tree: 8");
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_left_joystick(context, 90, 0, (0.5 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                context.wait_for_all_requests();
                enable_shiny_sound(context);
                pbf_press_button(context, BUTTON_B, (0.6 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_Y, (2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_right_joystick(context, 127, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                last_checked_tree = 9;
                check_tree_no_stop(env, context);

                //============ Tree 10=============//
                env.console.log("Checking tree: 10");
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_left_joystick(context, 0, 235, (0.5 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                context.wait_for_all_requests();
                enable_shiny_sound(context);
                pbf_press_button(context, BUTTON_B, (2.5 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_Y, (2.5 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                pbf_move_right_joystick(context, 127, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                last_checked_tree = 10;
                check_tree_no_stop(env, context);

                //============ Tree 11=============//
                env.console.log("Checking tree: 11");
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_left_joystick(context, 145, 0, (0.5 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                context.wait_for_all_requests();
                enable_shiny_sound(context);
                pbf_press_button(context, BUTTON_B, (1.8 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_Y, (1.8 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_right_joystick(context, 127, 255, (0.3 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                last_checked_tree = 11;
                break;
            case 3:
                //============ Tree 12=============//
                env.console.log("Checking tree: 12");
                pbf_move_left_joystick(context, 148, 255, 20, 20);
                change_mount(env.console, context, MountState::BRAVIARY_ON);
                pbf_press_button(context, BUTTON_B, (9.7 * TICKS_PER_SECOND), 20);
                pbf_press_button(context, BUTTON_Y, (2.2 * TICKS_PER_SECOND), 20);
                pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                pbf_move_left_joystick(context, 255, 127, 30, (0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
                pbf_move_right_joystick(context, 127, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                last_checked_tree = 12;
                check_tree_no_stop(env, context);

                //============ Tree 13=============//
                env.console.log("Checking tree: 13");
                pbf_move_left_joystick(context, 148, 0, 20, 20);
                change_mount(env.console, context, MountState::BRAVIARY_ON);
                context.wait_for_all_requests();
                enable_shiny_sound(context);
                pbf_press_button(context, BUTTON_B, (2 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_Y, (3 * TICKS_PER_SECOND), 0);
                pbf_move_right_joystick(context, 127, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                last_checked_tree = 13;
                check_tree_no_stop(env, context);

                //============ Tree 14=============//
                env.console.log("Checking tree: 14");
                pbf_move_left_joystick(context, 255, 155, 20, (0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
                change_mount(env.console, context, MountState::BRAVIARY_ON);
                context.wait_for_all_requests();
                enable_shiny_sound(context);
                pbf_move_left_joystick(context, 127, 0, (6.6 * TICKS_PER_SECOND), 20);
                pbf_move_right_joystick(context, 127, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                last_checked_tree = 14;
                check_tree_no_stop(env, context);

                //============ Tree 15=============//
                env.console.log("Checking tree: 15");
                pbf_move_left_joystick(context, 200, 0, 20, (0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
                change_mount(env.console, context, MountState::BRAVIARY_ON);
                context.wait_for_all_requests();
                enable_shiny_sound(context);
                pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_B, (1.7 * TICKS_PER_SECOND), 0);
                pbf_press_button(context, BUTTON_Y, (3 * TICKS_PER_SECOND), 0);
                pbf_move_right_joystick(context, 127, 255, (0.1 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                last_checked_tree = 15;
                break;
            }
        },
        {
            {battle_menu_detector},
        }
    );

    if (ret == 0){
        if (handle_battle(env, context)){
            env.console.log("Battle found before last tree in the path.");
            tree_counter.tree[last_checked_tree]++;
        }
    }else{
        // Check last tree
        if (check_tree(env, context)){
            tree_counter.tree[last_checked_tree]++;
        }
        env.console.log("Checked all trees in the path.");
    }

    return last_checked_tree;
}

void BurmyFinder::single_path(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t path, size_t last_tree, TreeCounter& tree_counter){
    env.console.log("Last tree was: " + std::to_string(last_tree));

    switch (path){
    case 0:
        if (last_tree < 1){
            env.console.log("Heading to tree 1");
            go_to_height_camp(env, context);
            pbf_move_left_joystick(context, 255, 110, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (7.2 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_Y, (4.5 * TICKS_PER_SECOND), 0);
            pbf_move_left_joystick(context, 180, 0, 20, (0.5 * TICKS_PER_SECOND));
            pbf_move_right_joystick(context, 127, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
            if (check_tree(env, context)){
                tree_counter.tree[1]++;
            }
        }

        if (last_tree < 2){
            env.console.log("Heading to tree 2");
            go_to_height_camp(env, context);
            pbf_move_left_joystick(context, 255, 147, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (6.6 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_Y, (4.4 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_PLUS, 20, (0.5 * TICKS_PER_SECOND));
            if (check_tree(env, context)){
                tree_counter.tree[2]++;
            }
        }

        if (last_tree < 3){
            env.console.log("Heading to tree 3");
            go_to_height_camp(env, context);
            pbf_move_left_joystick(context, 255, 158, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (9.5 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_Y, (4.5 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
            if (check_tree(env, context)){
                tree_counter.tree[3]++;
            }
        }
        break;
    case 1:
        if (last_tree < 5){
            env.console.log("Heading to Tree 5");
            go_to_height_camp(env, context);
            pbf_move_left_joystick(context, 240, 240, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (5.95 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (1.2 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (1.5 * TICKS_PER_SECOND));
            pbf_move_left_joystick(context, 0, 0, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
            pbf_move_right_joystick(context, 127, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
            if (check_tree(env, context)){
                tree_counter.tree[5]++;
            }
        }

        if (last_tree < 6){
            env.console.log("Heading to Tree 6");
            go_to_height_camp(env, context);
            pbf_move_left_joystick(context, 255, 235, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (8 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_Y, (4 * TICKS_PER_SECOND), 0);
            if (check_tree(env, context)){
                tree_counter.tree[6]++;
            }
        }
        break;

    case 2:
        if (last_tree < 8){
            env.console.log("Heading to Tree 8");
            go_to_height_camp(env, context);
            pbf_move_left_joystick(context, 173, 255, 20, (0.5 * TICKS_PER_SECOND));
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (8.8 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_Y, (4 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_PLUS, 20, 20);
            if (check_tree(env, context)){
                tree_counter.tree[8]++;
            }
        }

        if (last_tree < 9){
            env.console.log("Heading to Tree 9");
            go_to_height_camp(env, context);
            pbf_move_left_joystick(context, 163, 255, 20, (0.5 * TICKS_PER_SECOND));
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (11.7 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_Y, (3.8 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_PLUS, 20, 20);
            if (check_tree(env, context)){
                tree_counter.tree[8]++;
            }
        }

        if (last_tree < 10){
            env.console.log("Heading to Tree 10");
            go_to_height_camp(env, context);
            pbf_move_left_joystick(context, 218, 255, 20, (0.5 * TICKS_PER_SECOND));
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (10.4 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_Y, (3.8 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_PLUS, 20, 20);
            if (check_tree(env, context)){
                tree_counter.tree[8]++;
            }
        }

        if (last_tree < 11){
            env.console.log("Heading to Tree 11");
            go_to_height_camp(env, context);
            pbf_move_left_joystick(context, 255, 230, 20, (0.5 * TICKS_PER_SECOND));
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (11.4 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_Y, (3.8 * TICKS_PER_SECOND), 0);
            pbf_press_button(context, BUTTON_PLUS, 20, 20);
            if (check_tree(env, context)){
                tree_counter.tree[11]++;
            }
        }
        break;
    case 3:
        if (last_tree < 13){
            env.console.log("Heading to Tree 13");
            go_to_height_camp(env, context);
            pbf_move_left_joystick(context, 104, 255, 30, 30);
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (11 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (1.3 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (1.5 * TICKS_PER_SECOND));
            pbf_move_right_joystick(context, 127, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
            if (check_tree(env, context)){
                tree_counter.tree[13]++;
            }
        }

        if (last_tree < 14){
            env.console.log("Heading to Tree 14");
            go_to_height_camp(env, context);
            pbf_move_left_joystick(context, 108, 255, 20, 20);
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (8.7 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
            pbf_move_left_joystick(context, 140, 255, 20, 20);
            pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
            pbf_move_right_joystick(context, 140, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
            if (check_tree(env, context)){
                tree_counter.tree[14]++;
            }
        }
        if (last_tree < 15){
            env.console.log("Heading to Tree 15");
            go_to_height_camp(env, context);
            pbf_move_left_joystick(context, 160, 255, 30, 30);
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (6.35 * TICKS_PER_SECOND), 20);
            pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (1.5 * TICKS_PER_SECOND));
            pbf_move_left_joystick(context, 255, 135, 30, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
            pbf_move_right_joystick(context, 127, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
            if (check_tree(env, context)){
                tree_counter.tree[15]++;
            }
        }

        break;

    }

}

void BurmyFinder::run_iteration(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    TreeCounter& tree_counter,
    bool fresh_from_reset
){
    BurmyFinder_Descriptor::Stats& stats = env.current_stats<BurmyFinder_Descriptor::Stats>();
    stats.attempts++;
    env.update_stats();
    env.console.log("Starting route and shiny detection...");

    for (size_t c = 0; true; c++){
        context.wait_for_all_requests();
        auto snapshot = env.console.video().snapshot();
        if (is_pokemon_selection(env.console, snapshot)){
            break;
        }
        if (c >= 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to switch to Pokemon selection after 5 attempts.",
                env.console,
                std::move(snapshot)
            );
        }
        env.console.log("Not on Pokemon selection. Attempting to switch to it...", COLOR_ORANGE);
        pbf_press_button(context, BUTTON_X, 20, 230);
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

            for (int path = 0; path < 4; path++){
                size_t last_tree = grouped_path(env, context, path, tree_counter);
                context.wait_for_all_requests();
                single_path(env, context, path, last_tree, tree_counter);
                context.wait_for_all_requests();
            }
            //  End
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
            pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 2 * TICKS_PER_SECOND);
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

void BurmyFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    BurmyFinder_Descriptor::Stats& stats = env.current_stats<BurmyFinder_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    TreeCounter counters;

    bool fresh_from_reset = false;
    while (true){
        env.update_stats();
        counters.log(env.logger());
        send_program_status_notification(env, NOTIFICATION_STATUS);
        try{
            run_iteration(env, context, counters, fresh_from_reset);
        }catch (OperationFailedException& e){
            stats.errors++;
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
            fresh_from_reset = reset_game_from_home(
                env, env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



}
}
}
