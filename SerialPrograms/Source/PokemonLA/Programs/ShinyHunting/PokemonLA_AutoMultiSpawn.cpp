/*  Auto Multi-Spawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleStartDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleSpriteWatcher.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Inference/PokemonLA_WildPokemonFocusDetector.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/Programs/PokemonLA_TimeOfDayChange.h"
#include "PokemonLA/PokemonLA_WeatherAndTime.h"
#include "PokemonLA_AutoMultiSpawn.h"

#include <iostream>
#include <sstream>
#include <set>
#include <atomic>
#include <chrono>
#include <thread>
#include <iterator>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Pokemon;

const int MAX_DESPAWN_COUNT[] = {
    2 // Mirelands - Hippopotas
};

// The spawned pokemon we want to remove to advance a path
const std::set<std::string> TARGET_POKEMON[] = {
    {"hippopotas", "hippowdon"} // Mirelands - Hippopotas
};

// Possible nearby pokemon that we may focus onto near the spawn point.
const std::set<std::string> WILD_NEARBY_POKEMON[] = {
    {"stunky", "skuntank", "carnivine"} // Mirelands - Hippopotas
};

// Focus on a pokemon, change focus if possible until one of the target pokemon is found.
// Then thow the current item/pokemon to start a battle or catch/hit the pokemon.
// Currently it hardcoded to do maximum 4 focus change attempts before giving up.
// If it cannot find the target pokemon or OCR pokemon name reading fails, it returns
// an empty PokemonDetails.
// Otherwise, it returns the details of the pokemon thrown at.
PokemonDetails control_focus_to_throw(
    SingleSwitchProgramEnvironment& env,
    BotBaseContext& context, 
    const std::set<std::string>& target_pokemon,
    const std::set<std::string>& nearby_pokemon,
    Language language
){
    // A session that creates a new thread to send button commands to controller
    AsyncCommandSession session(context, env.console.logger(), env.realtime_dispatcher(), env.console.botbase());

    // First, let controller press ZL non-stop to start focusing on a pokemon
    session.dispatch([](BotBaseContext& context){
        pbf_press_button(context, BUTTON_ZL, 10000, 0);
    });

    // We try at most 4 focus change attempts
    int max_focus_change_attempt = 4;
    for(int focus_index = 0; focus_index <= max_focus_change_attempt; focus_index++){
        WildPokemonFocusDetector focus_detector(env.console.logger(), env.console);
        int ret = wait_until(
            env.console, context, std::chrono::seconds(3),
            {{focus_detector}}
        );
        
        if (ret < 0){
            env.log("Failed to focus on one pokemon.");
            session.stop_session_and_rethrow(); // Stop the commands
            return PokemonDetails();
        }

        // We have successfully focused on one pokemon.
        // Read its details and detect whether we can change focus.
        auto focused_frame = env.console.video().snapshot();
        PokemonDetails details = read_focused_wild_pokemon_info(env.console, env.console, focused_frame, language);
        bool can_change_focus = detect_change_focus(env.console, env.console, focused_frame);

        if (details.name_candidates.size() == 0){
            // Somehow the program detects a pokemon focus, but cannot read any names
            env.log("Warning: Focus on a pokemon but cannot read pokemon name.", COLOR_RED);
            session.stop_session_and_rethrow(); // Stop the commands
            return PokemonDetails();
        }

        { // log pokemon name candidates
            std::ostringstream os;
            std::copy(details.name_candidates.begin(), details.name_candidates.end(), std::ostream_iterator<std::string>(os, " "));
            env.log("Focused pokemon name candidates " + os.str());
        }

        bool found_nearby_pokemon = false;
        { // Filter out names that are not target pokemon
            std::set<std::string> filtered_names;
            for(const auto& name : details.name_candidates){
                if (target_pokemon.find(name) != target_pokemon.end()){
                    filtered_names.insert(name);
                }
                if (nearby_pokemon.find(name) != nearby_pokemon.end()){
                    found_nearby_pokemon = true;
                    env.log("Focused on a nearby pokemon " + name);
                }
            }
            details.name_candidates = std::move(filtered_names);
        }

        if (details.name_candidates.size() > 0){
            // We are focusing on a target pokemon
            // Press ZR to throw sth.
            // Dispatch a new series of commands that overwrites the last ones
            session.dispatch([](BotBaseContext& context){
                pbf_press_button(context, BUTTON_ZL | BUTTON_ZR, 30, 0);
                pbf_press_button(context, BUTTON_ZL, 50, 0);
            });

            env.log("Sending command to throw pokemon to start battle");
            session.wait();
            env.log("Waited for the throw pokemon command to finish");
            
            return details;
        }

        // Target pokemon not focused:

        if (can_change_focus == false){
            // We focused onto a pokemon that is not the target pokemon, and there is only one
            // pokemon that can be focused on.
            if (found_nearby_pokemon){
                // We focused on a nearby pokemon, so the player character's location is most likely correct,
                // just we are unlucky that the target pokemon is too far from our current position or the
                // player character is facing a wrong direction.
                env.log("Only one pokemon can be focused. No target pokemon nearby.");
            } else {
                // We focused on an unexpected pokemon. Maybe the trip is wrong.
                env.log("Focus on an unexpected pokemon.");
            }
            
            session.stop_session_and_rethrow();
            return PokemonDetails();
        }

        if (focus_index < max_focus_change_attempt){
            // We focused onto a pokemon that is not the target pokemon, but there are other pokemon that can be focused.
            // Press A to change focus.
            session.dispatch([](BotBaseContext& context){
                pbf_press_button(context, BUTTON_ZL | BUTTON_A, 30, 0);
                pbf_press_button(context, BUTTON_ZL, 10000, 0);
            });

            // Wait some time to let the button A press executed, the game focused on another pokemon
            context.wait_for(std::chrono::milliseconds(600));
        }
    }

    session.stop_session_and_rethrow();
    env.log("After four focus change attempts we cannot find a target pokemon");
    return PokemonDetails();
}




AutoMultiSpawn_Descriptor::AutoMultiSpawn_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:AutoMultiSpawn",
        STRING_POKEMON + " LA", "Auto Multi-Spawn",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/AutoMultiSpawn.md",
        "Advance a path in MultiSpawn shiny hunting method.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


AutoMultiSpawn::AutoMultiSpawn()
    : LANGUAGE("<b>Game Language</b>", Pokemon::PokemonNameReader::instance().languages(), true)
    ,SPAWN("<b>Spawn Point</b>:",
    {
        "Mirelands - Hippopotas"
    }, 0)
    , PATH(false, "<b>Multi-Spawn Path<b>:<br>e.g. \"A1|A1|A2|A2|A1|A1|A1|A2\".", "", "")
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(SPAWN);
    PA_ADD_OPTION(PATH);
    PA_ADD_OPTION(NOTIFICATIONS);
}

std::vector<int> parse_multispawn_path(SingleSwitchProgramEnvironment& env, const std::string& path, int max_num_despawn){
    std::vector<int> path_despawns;
    std::string raw_path = path + "|";
    for(size_t pos = 0, next_pos = 0; (next_pos = raw_path.find('|', pos)) != std::string::npos; pos = next_pos + 1){
        if (pos == next_pos){ // In the case it's jsut one "|"
            continue;
        }

        if (raw_path[pos] != 'A'){
            throw OperationFailedException(env.console, "Wrong Path string. Should have 'A' at pos " + std::to_string(pos));
        }
        raw_path[next_pos] = '\0';
        int despawn_count = strtol(raw_path.data() + pos+1, nullptr, 10);
        if (despawn_count <= 0 || despawn_count > max_num_despawn){
            throw OperationFailedException(env.console, "Wrong Path string. Invalid number " + std::to_string(despawn_count) + " at pos " + std::to_string(pos+1));
        }
        path_despawns.push_back(despawn_count);
    }

    return path_despawns;
}


void AutoMultiSpawn::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    const int max_num_despawn = MAX_DESPAWN_COUNT[SPAWN];

    // Parse input path string:
    std::vector<int> path_despawns = parse_multispawn_path(env, PATH.get(), max_num_despawn);
    std::vector<TimeOfDay> path_times;
    // We + 1 here because we need one more time change to read the desired target pokemon at the end (usually a shiny alpha)
    for(size_t i = 0; i < path_despawns.size() + 1; i++){
        path_times.push_back(i % 2 == 0 ? TimeOfDay::MORNING : TimeOfDay::MIDDAY);
    }
    
    {
        std::ostringstream os;
        for(size_t i = 0; i < path_despawns.size(); i++){
            if (i >0){
                os << '|';
            }
            // TODO: if murkrow path is added, need to output 'N'
            os << 'A' << path_despawns[i] << '(' << timeOfDayOneLetter(path_times[i]) << ')';
        }
        env.log("The path is: " + os.str());
    }
    
    // change_time_of_day(TimeOfDay.MORNING);

    for(size_t i = 0; i < path_despawns.size(); i++){
        // - Teleport to a camp
        // - From camp, go to the spawn point
        // - Battle the pokemon there to remove them
        // - Teleport back to camp
        // - Got to tent to change time of day
        advance_one_path_step(env, context, path_despawns[i], path_times[i], path_times[i+1]);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

void AutoMultiSpawn::advance_one_path_step(
    SingleSwitchProgramEnvironment& env,
    BotBaseContext& context,
    size_t num_to_despawn,
    TimeOfDay cur_time,
    TimeOfDay next_time
){

    // Switch to pokemon selection so that we can press X to throw a pokemon out to start a battle
    for (size_t c = 0; true; c++){
        context.wait_for_all_requests();
        if (is_pokemon_selection(env.console, env.console.video().snapshot())){
            break;
        }
        if (c >= 5){
            throw OperationFailedException(env.console, "Failed to switch to Pokemon selection after 5 attempts.");
        }
        env.console.log("Not on Pokemon selection. Attempting to switch to it...", COLOR_ORANGE);
        pbf_press_button(context, BUTTON_X, 20, 230);
    }
    
    // We try at most three battles to remove pokemon
    size_t already_removed_pokemon = 0;
    size_t remained_to_remove = num_to_despawn;
    for(int i = 0; i < 3; i++){
        // Go to spawn point to start a battle, remove some pokemon, then return to camp.
        size_t num_pokemon_removed = one_battle_to_remove_pokemon(env, context, remained_to_remove);
        already_removed_pokemon += num_pokemon_removed;
        env.log("Removed " + std::to_string(num_pokemon_removed) + " via battle, total "
             + std::to_string(already_removed_pokemon) + " pokemon rmoved, target total pokemon to remove: " + std::to_string(num_to_despawn));
        if (already_removed_pokemon > num_to_despawn){
            throw OperationFailedException(env.console, "Removed more pokemon than required. Removed "
                 + std::to_string(already_removed_pokemon) + " while target is " + std::to_string(num_to_despawn));
        }

        remained_to_remove -= num_pokemon_removed;

        // XXX
        // Check weather. If weather does not match the weather at the beginning of the path step, then
        // weather change refreshed the spawn. This advance is broken.

        if (remained_to_remove == 0){
            break;
        }
    }
    if (remained_to_remove > 0){
        throw OperationFailedException(env.console, "After trying to start three battles, cannot remove enough pokemon.");
    }

    // All pokemon removed.
    // Now go to tent to change time of day.
    change_time_of_day_at_tent(env.console, context, next_time, Camp::MIRELANDS_MIRELANDS);
}

// From camp, go to spawn, do one battle to remove pokemon. If no error, return camp
size_t AutoMultiSpawn::one_battle_to_remove_pokemon(SingleSwitchProgramEnvironment& env, BotBaseContext& context, size_t num_to_despawn){
    // From camp fly to the spawn point, focus on a target pokemon and start a battle
    auto go_to_spawn_point_and_start_battle = [&]() -> PokemonDetails {
        goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Mirelands_Mirelands);
        change_mount(env.console, context, MountState::BRAVIARY_ON);
        pbf_wait(context, 40);
        
        // Move to spawn location on Braviary
        pbf_move_left_joystick(context, 255, 165, 150, 0); // 170
        pbf_press_button(context, BUTTON_B, 200, 10);
        pbf_mash_button(context, BUTTON_B, 1500); // 1450

        // Descend down from the air:
        pbf_press_button(context, BUTTON_PLUS, 20, 150); // jump down from Braviary
        for(int i = 0; i < 2; i++){
            pbf_press_button(context, BUTTON_PLUS, 20, 50); // Call back Braviary to stop falling
            pbf_press_button(context, BUTTON_PLUS, 20, 150); // fall down again
        }
        
        // Move forward on foot
        pbf_move_left_joystick(context, 128, 0, 150, 0);

        context.wait_for_all_requests();
        
        return control_focus_to_throw(env, context, TARGET_POKEMON[SPAWN], WILD_NEARBY_POKEMON[SPAWN], LANGUAGE);
    };

    // Try to go to spawn point and focus on one pokemon
    PokemonDetails focused_pokemon;
    const size_t num_tries = 5;
    for(size_t i = 0; i < num_tries; i++){
        focused_pokemon = go_to_spawn_point_and_start_battle();
        env.log("Finish trying to start a battle");
        if (focused_pokemon.name_candidates.size() > 0){
            // We found one
            env.log("Focused on one pokemon and starting battle.");
            break;
        }
        env.log("Cannot focus on any pokemon. Retry.");
        // TODO: escape routine may scare wild pokemon. A better way is to reset and load from the backup save?
        // or load from a previous save?
        goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Mirelands_Mirelands);
        // TODO: May need to reset time of day here
    }

    if (focused_pokemon.name_candidates.size() == 0){
        throw OperationFailedException(env.console, "Cannot focus on a pokemon after going to the spawn point  " + std::to_string(num_tries) + " times");
    }
    
    // We have found a target pokemon and initiated a pokemon battle

    // BattleStartDetector battle_start_detector(env.console.logger(), env.console);
    BattleSpriteWatcher battle_sprite_watcher(env.console.logger(), env.console.overlay());
    env.log("Starting watching for battle menu or battle ending");

    bool battle_starting = true;
    size_t num_initial_pokemon = 0;
    while(true){
        const bool stop_on_detected = true;
        BattleMenuDetector battle_menu_detector(env.console.logger(), env.console, stop_on_detected);
        ArcPhoneDetector arc_phone_detector(env.console, env.console, std::chrono::milliseconds(100), stop_on_detected);

        std::vector<PeriodicInferenceCallback> callbacks = {
            {arc_phone_detector},
            {battle_menu_detector}
        };
        if (battle_starting){
            callbacks.emplace_back(battle_sprite_watcher);
        }

        int ret = wait_until(
            env.console, context, std::chrono::seconds(30), callbacks
        );

        if (ret < 0){
            throw OperationFailedException(env.console, "Cannot detect a battle after 30 seconds");
        }

        if (battle_starting){
            for(bool appeared: battle_sprite_watcher.sprites_appeared()){
                num_initial_pokemon += appeared;
            }

            battle_starting = false;
        }

        if (ret == 0){  // battle ends
            env.log("Battle ends");
            size_t num_removed_pokemon = 0;
            if (num_initial_pokemon == 0){
                // battle ends but no sprite detected. So there is only one pokmeon in the battle and it immediately fled.
                num_removed_pokemon = 1;
            } else {
                // More than one pokemon attended the battle and they all fled.
                // We think our pokemon is strong enough to defeat all the wild pokemon in the battle, so it is not possible
                // that the battle ends because we lost.
                num_removed_pokemon = num_initial_pokemon;
            }
            return num_removed_pokemon;
        }

        env.log("Found battle menu");
        // now in battle menu
        const auto sprites_remain = battle_sprite_watcher.detect_sprites(*(env.console.video().snapshot().frame));
        size_t num_sprites_remain = 0;
        for(bool remain : sprites_remain){
            num_sprites_remain += remain;
        }

        size_t num_removed_pokemon = num_initial_pokemon - num_sprites_remain;
        if (num_removed_pokemon > num_to_despawn){
            // Oh no, we removed more than needed. Return immediately
            return num_removed_pokemon;
        } else if (num_removed_pokemon < num_to_despawn){
            // Mash A to remove pokemon
            // We assume we have enough PP
            pbf_mash_button(context, BUTTON_A, 500);
            // Wait for three seconds, then check for the next turn of the battle (or battle ends)
            context.wait_for(std::chrono::seconds(3));
            continue;
        }
        
        // num_removed_pokemon == num_to_despawn:
        // We removed exactly what we need.
        // Escape battle
        env.log("Running from battle...");
        pbf_press_button(context, BUTTON_B, 20, 225);
        pbf_press_button(context, BUTTON_A, 20, 100);
        context.wait_for_all_requests();
        ArcPhoneDetector escape_detector(env.console, env.console, std::chrono::milliseconds(100), stop_on_detected);
        ret = wait_until(
            env.console, context, std::chrono::seconds(30), {{escape_detector}}
        );
        if (ret < 0){
            throw OperationFailedException(env.console, "Cannot detect end of battle when escaping");
        }
        goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Mirelands_Mirelands);
        return num_removed_pokemon;
    }
}


}
}
}
