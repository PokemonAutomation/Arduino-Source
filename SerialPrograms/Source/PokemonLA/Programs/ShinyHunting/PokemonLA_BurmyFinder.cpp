/*  Burmy Hunter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <array>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Inference/PokemonLA_StatusInfoScreenDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/ShinyHunting/PokemonLA_BurmyFinder.h"
#include "PokemonLA/Programs/PokemonLA_LeapPokemonActions.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


BurmyFinder_Descriptor::BurmyFinder_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:Burmy Hunter",
        STRING_POKEMON + " LA", "Burmy Hunter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/BurmyHunter.md",
        "Check nearby trees for a possible Shiny, Alpha or Alpha Shiny Burmy",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


BurmyFinder::BurmyFinder(const BurmyFinder_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE("<b>Game Language</b>", Pokemon::PokemonNameReader::instance().languages(), true)
    , STOP_ON(
        "<b>Stop On:</b>",
        {
            "Shiny",
            "Alpha",
            "Shiny & Alpha",
            "Stop on any non regular",
        },
        0
    )
    , EXIT_METHOD(
        "<b>Exit Battle Method:</b>",
        {
            "Run Away",
            "Mash A to Kill",
        },
        0
    )
    , SHINY_DETECTED_ENROUTE(
        "Enroute Shiny Action",
        "This applies if a shiny is detected while traveling in the overworld.",
        "0 * TICKS_PER_SECOND"
    )
    , MATCH_DETECTED_OPTIONS(
      "Match Action",
      "What to do when a Burmy is found that matches the \"Stop On\" parameter.",
      "0 * TICKS_PER_SECOND")

    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &MATCH_DETECTED_OPTIONS.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(STOP_ON);
    PA_ADD_OPTION(EXIT_METHOD);
    PA_ADD_OPTION(SHINY_DETECTED_ENROUTE);
    PA_ADD_OPTION(MATCH_DETECTED_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}


class BurmyFinder::Stats : public StatsTracker{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , trees(m_stats["Trees"])
        , errors(m_stats["Errors"])
        , found(m_stats["Found"])
        , enroute_shinies(m_stats["Enroute Shinies"])
        , tree_alphas(m_stats["Tree Alphas"])
        , tree_shinies(m_stats["Tree Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Trees");
        m_display_order.emplace_back("Errors", true);
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
    std::atomic<uint64_t>& found;
    std::atomic<uint64_t>& enroute_shinies;
    std::atomic<uint64_t>& tree_alphas;
    std::atomic<uint64_t>& tree_shinies;

};

std::unique_ptr<StatsTracker> BurmyFinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


struct BurmyFinder::TreeCounter{
    std::array<uint64_t, 10> tree = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    
    void log(Logger& logger) const{
        std::ostringstream oss;
        for(size_t i = 0; i < 10; i++){
            if (i != 0){
                oss << " - ";
            }
            oss << "Tree" << i << ": " << tostr_u_commas(tree[i]);
        }
        logger.log(oss.str());
    }
};

bool BurmyFinder::handle_battle(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    PokemonDetails pokemon = get_pokemon_details(env.console, context, LANGUAGE);

    pbf_press_button(context, BUTTON_B, 20, 225);

    context.wait_for_all_requests();

    if (pokemon.name_candidates.find("burmy") == pokemon.name_candidates.end()){
        env.console.log("Not a burmy. Leaving battle.");
        exit_battle(env.console, context, EXIT_METHOD == 1);
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
    case 0: //  Shiny
        is_match = pokemon.is_shiny;
        break;
    case 1: //  Alpha
        is_match = pokemon.is_alpha;
        break;
    case 2: //  Shiny and Alpha
        is_match = pokemon.is_alpha && pokemon.is_shiny;
        break;
    case 3: //  Shiny or Alpha
        is_match = pokemon.is_alpha || pokemon.is_shiny;
        break;
    }

    if (pokemon.is_alpha || pokemon.is_shiny){
        on_match_found(env, env.console, context, MATCH_DETECTED_OPTIONS, is_match);
    }

    exit_battle(env.console, context, EXIT_METHOD == 1);

    return true;
}


bool BurmyFinder::check_tree(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    disable_shiny_sound(context);
    bool battle_found = check_tree_or_ore_for_battle(env.console, context);
    env.stats<Stats>().trees++;
    env.update_stats();

    context.wait_for_all_requests();

    bool ret = false;
    if (battle_found){
        ret = handle_battle(env, context);
    }

    enable_shiny_sound(context);

    return ret;
}

void BurmyFinder::check_tree_no_stop(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    // Throw pokemon
    pbf_press_button(context, BUTTON_ZR, (0.5 * TICKS_PER_SECOND), 1.5 * TICKS_PER_SECOND);
    env.stats<Stats>().trees++;
    env.update_stats();
}

void BurmyFinder::disable_shiny_sound(BotBaseContext& context){
    context.wait_for_all_requests();
    m_enable_shiny_sound.store(false, std::memory_order_release);
}
void BurmyFinder::enable_shiny_sound(BotBaseContext& context){
    context.wait_for_all_requests();
    m_enable_shiny_sound.store(true, std::memory_order_release);
}

void BurmyFinder::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context, TreeCounter& tree_counter){
    Stats& stats = env.stats<Stats>();
    stats.attempts++;
    env.update_stats();
    env.console.log("Starting route and shiny detection...");

    for (size_t c = 0; true; c++){
        context.wait_for_all_requests();
        if (is_pokemon_selection(env.console, env.console.video().snapshot().frame)){
            break;
        }
        if (c >= 5){
            throw OperationFailedException(env.console, "Failed to switch to Pokemon selection after 5 attempts.");
        }
        env.console.log("Not on Pokemon selection. Attempting to switch to it...", COLOR_ORANGE);
        pbf_press_button(context, BUTTON_X, 20, 230);
    }

    float shiny_coefficient = 1.0;

    ShinySoundDetector shiny_detector(env.console.logger(), env.console, [&](float error_coefficient) -> bool{
        //  Warning: This callback will be run from a different thread than this function.
        if (!m_enable_shiny_sound.load()){
            return false;
        }
        stats.enroute_shinies++;
        shiny_coefficient = error_coefficient;
        return on_shiny_callback(env, env.console, SHINY_DETECTED_ENROUTE, error_coefficient);
    });

    goto_camp_from_jubilife(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);

    size_t cur_tree = 0;
    int ret = run_until(
        env.console, context,
        [&](BotBaseContext& context){
            //  Tree 0
            env.console.log("Heading to Tree 0");
            pbf_move_left_joystick(context, 148, 255, 20, 20);
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (11.8 * TICKS_PER_SECOND), 20);
            pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (1.5 * TICKS_PER_SECOND));
            pbf_move_left_joystick(context, 255, 127, 30, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
            pbf_move_right_joystick(context, 127, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));

            if (check_tree(env, context)){
                tree_counter.tree[0]++;
            }

            //Tree 1
            env.console.log("Heading to Tree 1");
            goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
            pbf_move_left_joystick(context, 104, 255, 30, 30);
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (11 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (1.3 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (1.5 * TICKS_PER_SECOND));
            pbf_move_right_joystick(context, 127, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));

            if (check_tree(env, context)){
                tree_counter.tree[1]++;
            }


            //Tree 2
            env.console.log("Heading to Tree 2");
            goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
            pbf_move_left_joystick(context, 108, 255, 20, 20);
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (8.7 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
            pbf_move_left_joystick(context, 140, 255, 20, 20);
            pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
            pbf_move_right_joystick(context, 140, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
            context.wait_for_all_requests();

            if (check_tree(env, context)){
                tree_counter.tree[2]++;
            }


            //Tree 3
            env.console.log("Heading to Tree 3");
            goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
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
                tree_counter.tree[3]++;
            }

            //Tree 4
            env.console.log("Heading to Tree 4");
            goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
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
                tree_counter.tree[4]++;
            }

            //Tree 5
            env.console.log("Heading to Tree 5");
            goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
            //pbf_move_left_joystick(context, 255, 165, 20, (0.5 * TICKS_PER_SECOND));
            pbf_move_left_joystick(context, 255, 158, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_press_button(context, BUTTON_B, (6.4 * TICKS_PER_SECOND), (1 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_PLUS, 20, (1.5 * TICKS_PER_SECOND));

            pbf_move_left_joystick(context, 255, 130, 20, (0.5 * TICKS_PER_SECOND));
            pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
            pbf_move_right_joystick(context, 127, 255, (0.15 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
            
            if (check_tree(env, context)){
                tree_counter.tree[5]++;
            }

            // Tree 6-9
            env.console.log("Heading to Tree 6-9");
            goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
            const bool stop_on_detected = true;
            BattleMenuDetector battle_menu_detector(env.console, env.console, stop_on_detected);
            int ret = run_until(
                env.console, context,
                [&](BotBaseContext& context){
                    // === Fly to the first tree on trip ===
                    cur_tree = 6;
                    pbf_move_left_joystick(context, 255, 90, 20, (0.5 * TICKS_PER_SECOND));
                    pbf_press_button(context, BUTTON_ZL, 20, (0.5 * TICKS_PER_SECOND));
                    change_mount(env.console, context, MountState::BRAVIARY_ON);
                    pbf_press_button(context, BUTTON_B, (5 * TICKS_PER_SECOND), 0);
                    pbf_press_button(context, BUTTON_Y, (4.2 * TICKS_PER_SECOND), 0);
                    pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));

                    // Now on ground, move towards the tree
                    // change_mount(env.console, context, MountState::WYRDEER_OFF);
                    pbf_move_left_joystick(context, 0, 70, 20, (0.3 * TICKS_PER_SECOND));
                    // Change camera to face what player character faces
                    pbf_press_button(context, BUTTON_ZL, 20, (0.4 * TICKS_PER_SECOND));
                    // Move camera down
                    pbf_move_right_joystick(context, 128, 255, (0.2 * TICKS_PER_SECOND), (0.5 * TICKS_PER_SECOND));
                    
                    // Now we should be on ground
                    disable_shiny_sound(context);
                    check_tree_no_stop(env, context);

                    // === Fly to the second tree on trip ===
                    // Mount on Braviary
                    pbf_press_button(context, BUTTON_PLUS, 20, 100);
                    enable_shiny_sound(context);

                    // Now face towards the next tree
                    pbf_move_left_joystick(context, 255, 200, 1 * TICKS_PER_SECOND, 0);
                    pbf_press_button(context, BUTTON_B, 60, 0);
                    pbf_press_button(context, BUTTON_Y, (2.3 * TICKS_PER_SECOND), 0);
                    // Get off Braviary
                    pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                    // Move camera down
                    pbf_move_right_joystick(context, 128, 255, (0.1 * TICKS_PER_SECOND), (0.3 * TICKS_PER_SECOND));
                    
                    // Now we should be on ground
                    disable_shiny_sound(context);
                    check_tree_no_stop(env, context);
                    cur_tree = 7;
                    
                    // === Fly to the third tree on trip ===
                    // Mount on Braviary
                    pbf_press_button(context, BUTTON_PLUS, 20, 100);
                    enable_shiny_sound(context);

                    // Now face towards the next tree
                    pbf_move_left_joystick(context, 255, 130, 1 * TICKS_PER_SECOND, 0);
                    pbf_press_button(context, BUTTON_B, 80, 0);
                    pbf_press_button(context, BUTTON_Y, (2.2 * TICKS_PER_SECOND), 0);
                    // Get off Braviary
                    pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                    // Move camera down
                    pbf_move_right_joystick(context, 128, 255, (0.1 * TICKS_PER_SECOND), (0.3 * TICKS_PER_SECOND));

                    // Now we should be on ground
                    disable_shiny_sound(context);
                    check_tree_no_stop(env, context);
                    cur_tree = 8;

                    // === Fly to the fifth tree ===
                    // Mount on Braviary
                    pbf_press_button(context, BUTTON_PLUS, 20, 100);
                    enable_shiny_sound(context);

                    // Now face towards the next tree
                    pbf_move_left_joystick(context, 0, 60, 1 * TICKS_PER_SECOND, 0);
                    pbf_press_button(context, BUTTON_B, 90, 0);
                    pbf_press_button(context, BUTTON_Y, (2.3 * TICKS_PER_SECOND), 0);
                    // Get off Braviary
                    pbf_press_button(context, BUTTON_PLUS, 20, (1 * TICKS_PER_SECOND));
                    // Move camera down
                    pbf_move_right_joystick(context, 128, 255, (0.1 * TICKS_PER_SECOND), (0.3 * TICKS_PER_SECOND));
                    // Now we should be on ground
                },
                {
                    {battle_menu_detector},
                }
            );
            
            if (ret == 0){
                env.log("Found battle during river-side traversal.");
                if (handle_battle(env, context)){
                    tree_counter.tree[cur_tree]++;
                }
                
            } else{
                // Check last tree
                if (check_tree(env, context)){
                    tree_counter.tree[9]++;
                }
            }

            //  End
            goto_camp_from_overworld(env, env.console, context);
            goto_professor(env.console, context, Camp::FIELDLANDS_FIELDLANDS);
        },
        {{shiny_detector}}
    );
    shiny_detector.throw_if_no_sound();
    if (ret == 0){
        on_shiny_sound(env, env.console, context, SHINY_DETECTED_ENROUTE, shiny_coefficient);
    }

    from_professor_return_to_jubilife(env, env.console, context);
}

void BurmyFinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    TreeCounter counters;

    while (true){
        env.update_stats();
        counters.log(env.logger());
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS,
            env.program_info(),
            "",
            stats.to_str()
        );
        try{
            run_iteration(env, context, counters);
        }catch (OperationFailedException&){
            stats.errors++;
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }
    }

    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
}



}
}
}
