/*  Auto Multi-Spawn
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <set>
//#include <atomic>
#include <chrono>
//#include <thread>
#include <iterator>
#include <sstream>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleSpriteWatcher.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Inference/PokemonLA_WildPokemonFocusDetector.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Programs/PokemonLA_BattleRoutines.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/Programs/PokemonLA_TimeOfDayChange.h"
#include "PokemonLA/PokemonLA_WeatherAndTime.h"
#include "PokemonLA_AutoMultiSpawn.h"

#include <iostream>
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
// 
// If a target pokemon is focused, the function returns immediately when the item/pokemon is throw, with
// return value: (true, target pokemon details).
// If it fails to find any pokemon, return (false, empty details).
// If it can focus on one pokemon but cannot find a target pokemon, return (true, empty details).
//
// Currently it hardcoded to do maximum 4 focus change attempts before giving up.
// If it cannot find the target pokemon or OCR pokemon name reading fails, it returns
// an empty PokemonDetails.
// Otherwise, it returns the details of the pokemon thrown at.
std::pair<bool, PokemonDetails> control_focus_to_throw(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    const std::set<std::string>& target_pokemon,
    const std::set<std::string>& nearby_pokemon,
    Language language
){
    // A session that creates a new thread to send button commands to controller
    AsyncCommandSession<ProController> session(
        context,
        env.console.logger(),
        env.realtime_dispatcher(),
        env.console.controller<ProController>()
    );

    // First, let controller press ZL non-stop to start focusing on a pokemon
    session.dispatch([](ProControllerContext& context){
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
            return std::make_pair(false, PokemonDetails());
        }

        // We have successfully focused on one pokemon.
        // Wait for a while so that the pokemon name is fully rendered, to avoid mis-read pokemon name.
        context.wait_for(std::chrono::milliseconds(300));
        // Read its details and detect whether we can change focus.
        auto focused_frame = env.console.video().snapshot();
        PokemonDetails details = read_focused_wild_pokemon_info(env.console, env.console, focused_frame, language);
        bool can_change_focus = detect_change_focus(env.console, env.console, focused_frame);

        if (details.name_candidates.size() == 0){
            // Somehow the program detects a pokemon focus, but cannot read any names
            env.log("Warning: Focus on a pokemon but cannot read pokemon name.", COLOR_RED);
            session.stop_session_and_rethrow(); // Stop the commands
            return std::make_pair(false, PokemonDetails());
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
            session.dispatch([](ProControllerContext& context){
                pbf_press_button(context, BUTTON_ZL | BUTTON_ZR, 30, 0);
                pbf_press_button(context, BUTTON_ZL, 50, 0);
            });

            env.log("Sending command to throw pokemon to start battle");
            session.wait();
            env.log("Waited for the throw pokemon command to finish");
            session.stop_session_and_rethrow();
            context.wait_for_all_requests();
            return std::make_pair(true, details);
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
            }else{
                // We focused on an unexpected pokemon. Maybe the trip is wrong.
                env.log("Focus on an unexpected pokemon.");
            }
            
            session.stop_session_and_rethrow();
            return std::make_pair(true, PokemonDetails());
        }

        if (focus_index < max_focus_change_attempt){
            // We focused onto a pokemon that is not the target pokemon, but there are other pokemon that can be focused.
            // Press A to change focus.
            session.dispatch([](ProControllerContext& context){
                pbf_press_button(context, BUTTON_ZL | BUTTON_A, 30, 0);
                pbf_press_button(context, BUTTON_ZL, 10000, 0);
            });

            // Wait some time to let the button A press executed, the game focused on another pokemon
            context.wait_for(std::chrono::milliseconds(600));
        }
    }

    session.stop_session_and_rethrow();
    env.log("After four focus change attempts we cannot find a target pokemon");
    return std::make_pair(true, PokemonDetails());
}




AutoMultiSpawn_Descriptor::AutoMultiSpawn_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:AutoMultiSpawn",
        STRING_POKEMON + " LA", "Auto Multi-Spawn",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/AutoMultiSpawn.md",
        "Advance a path in MultiSpawn shiny hunting method.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}


AutoMultiSpawn::AutoMultiSpawn()
    : LANGUAGE(
        "<b>Game Language</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , SPAWN(
        "<b>Spawn Point</b>:",
        {
            {MultiSpawn::MirelandsHippopotas, "mirelands-hippopotas", "Mirelands - Hippopotas"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        MultiSpawn::MirelandsHippopotas
    )
    , PATH(
        false,
        "<b>Multi-Spawn Path<b>:<br>e.g. \"A1|A1|A2|A2|A1|A1|A1|A2\".",
        LockMode::LOCK_WHILE_RUNNING,
        "", ""
    )
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
        if (pos == next_pos){ // In the case it's just one "|"
            continue;
        }

        if (raw_path[pos] != 'A'){
            throw InternalProgramError(
                &env.console.logger(),
                PA_CURRENT_FUNCTION,
                "Wrong Path string. Should have 'A' at pos " + std::to_string(pos)
            );
        }
        raw_path[next_pos] = '\0';
        int despawn_count = static_cast<int>(strtol(raw_path.data() + pos+1, nullptr, 10));
        if (despawn_count <= 0 || despawn_count > max_num_despawn){
            throw InternalProgramError(
                &env.console.logger(),
                PA_CURRENT_FUNCTION,
                "Wrong Path string. Invalid number " + std::to_string(despawn_count) + " at pos " + std::to_string(pos+1)
            );
        }
        path_despawns.push_back(despawn_count);
    }

    return path_despawns;
}


void AutoMultiSpawn::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    MultiSpawn spawn = SPAWN;
    const int max_num_despawn = MAX_DESPAWN_COUNT[(size_t)spawn];

    // Parse input path string:
    std::vector<int> path_despawns = parse_multispawn_path(env, PATH, max_num_despawn);
    std::vector<TimeOfDay> path_times;
    // We + 1 here because we need one more time change to read the desired target pokemon at the end (usually a shiny alpha)
    for(size_t i = 0; i < path_despawns.size() + 1; i++){
        // TODO: if murkrow path is added, need to output 'N'
        path_times.push_back(i % 2 == 0 ? TimeOfDay::MORNING : TimeOfDay::MIDDAY);
    }
    
    {
        std::ostringstream os;
        for(size_t i = 0; i < path_despawns.size(); i++){
            if (i >0){
                os << '|';
            }
            os << 'A' << path_despawns[i] << '(' << timeOfDayOneLetter(path_times[i]) << ')';
        }
        env.log("The path is: " + os.str());
    }
    
    goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Mirelands_Mirelands);
    change_time_of_day_at_tent(env.console, context, path_times[0], Camp::MIRELANDS_MIRELANDS);

    for(size_t iStep = 0; iStep < path_despawns.size(); iStep++){
        // - Teleport to a camp
        // - From camp, go to the spawn point
        // - Battle the pokemon there to remove them
        // - Teleport back to camp
        // - Got to tent to change time of day
        advance_one_path_step(env, context, max_num_despawn, path_despawns[iStep], path_times[iStep], path_times[iStep+1]);

        std::ostringstream os;
        for(size_t jStep = 0; jStep < path_despawns.size(); jStep++){
            os << 'A' << path_despawns[jStep] << '(' << timeOfDayOneLetter(path_times[jStep]) << ")|";
            if (iStep == jStep){
                os << '*';
            }
        }
        env.log("Path Progress: " + os.str(), COLOR_CYAN);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

void AutoMultiSpawn::advance_one_path_step(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    size_t num_spawned_pokemon,
    size_t num_to_despawn,
    TimeOfDay cur_time,
    TimeOfDay next_time
){

    // Switch to pokemon selection so that we can press X to throw a pokemon out to start a battle
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

    goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Mirelands_Mirelands);
    
    // We try at most three battles to remove pokemon
    size_t already_removed_pokemon = 0;
    size_t remained_to_remove = num_to_despawn;
    size_t pokemon_left = num_spawned_pokemon;
    for(int i = 0; i < 3; i++){
        // Go to spawn point to start a battle, remove some pokemon, then return to camp.
        size_t num_pokemon_removed = try_one_battle_to_remove_pokemon(env, context, pokemon_left, remained_to_remove);
        already_removed_pokemon += num_pokemon_removed;
        env.log(
            "Removed " + std::to_string(num_pokemon_removed) + " via battle, total "
             + std::to_string(already_removed_pokemon) + " pokemon removed, target total pokemon to remove: " + std::to_string(num_to_despawn)
         );
        if (already_removed_pokemon > num_to_despawn){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Removed more pokemon than required. Removed "
                + std::to_string(already_removed_pokemon) + " while target is " + std::to_string(num_to_despawn),
                env.console
            );
        }

        remained_to_remove -= num_pokemon_removed;
        pokemon_left -= num_pokemon_removed;

        // XXX
        // Check weather. If weather does not match the weather at the beginning of the path step, then
        // weather change refreshed the spawn. This advance is broken.

        if (remained_to_remove == 0){
            break;
        }
    }
    if (remained_to_remove > 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "After trying to start three battles, cannot remove enough pokemon.",
            env.console
        );
    }

    // All pokemon removed. Now go to tent to change time of day.
    change_time_of_day_at_tent(env.console, context, next_time, Camp::MIRELANDS_MIRELANDS);
}

// From camp, go to spawn, do one battle to remove pokemon. If no error, return camp
size_t AutoMultiSpawn::try_one_battle_to_remove_pokemon(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    size_t num_left,
    size_t num_to_despawn
){
    // Try to go to spawn point and focus on one pokemon
    PokemonDetails focused_pokemon;
    const size_t num_tries = 5;
    for(size_t i = 0; i < num_tries; i++){
        // From camp go to the spawn point, try focusing on one pokemon.
        // Return the pokemon details if found the target pokemon. Otherwise if cannot find one, return empty details.
        focused_pokemon = go_to_spawn_point_and_try_focusing_pokemon(env, context, num_left);

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
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Cannot focus on a pokemon after going to the spawn point  " + std::to_string(num_tries) + " times",
            env.console
        );
    }
    
    // We have found a target pokemon and initiated a pokemon battle
    env.log("Now waiting for pokemon battle to begin...");
    BattleSpriteWatcher battle_sprite_watcher(env.console.logger(), env.console.overlay());
    wait_until(
        env.console, context, std::chrono::seconds(3), {{battle_sprite_watcher}}
    );
    env.log("Waited for 3 sec. Now start detecting battle menu or battle ends.");

    bool battle_starting = true;
    size_t num_initial_sprites = 0;
    size_t num_removed_pokemon = 0;
    bool battle_menu_detectd = false;
    size_t cur_move = 0;
    while(true){
        const bool stop_on_detected = true;
        BattleMenuDetector battle_menu_detector(env.console.logger(), env.console, stop_on_detected);
        ArcPhoneDetector arc_phone_detector(env.console, env.console, std::chrono::milliseconds(100), stop_on_detected);

        std::vector<PeriodicInferenceCallback> callbacks = {
            {arc_phone_detector},
            {battle_menu_detector}
        };
        int seconds_to_wait = 60;
        if (battle_starting){
            callbacks.emplace_back(battle_sprite_watcher);
            // When battle starts, assume your pokemon is the fastest, your turn should
            // arrive very quickly.
            seconds_to_wait = 30;
        }

        int ret = wait_until(
            env.console, context, std::chrono::seconds(seconds_to_wait), callbacks
        );

        if (ret < 0){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Cannot detect a battle after 30 seconds.",
                env.console
            );
        }

        if (battle_starting){
            for(bool appeared: battle_sprite_watcher.sprites_appeared()){
                num_initial_sprites += appeared;
            }

            env.log("Battle started, detected battle sprite count: " + std::to_string(num_initial_sprites) +
                    ", number to despawn: " + std::to_string(num_to_despawn));
            battle_starting = false;
        }

        if (ret == 0){  // battle ends
            env.log("Battle ends");
            if (num_initial_sprites == 0 || battle_menu_detectd == false){
                // num_initial_sprites == 0: battle ends but no sprite detected. So there is only one pokmeon in the battle and it is removed
                // battle_menu_detectd == false: we don't go to battle menu before battle ends. This means there is only skittish pokemon and
                // it/they flee. Because skittish pokemon cannot be multi-battled, so there is only one skitish pokemon.
                //
                // Note we need `battle_menu_detectd == false` condition because if battle ends without entering battle menu, battle sprite
                // watcher will have lots of false positives when the upper and lower dark borders gradually fade away.
                num_removed_pokemon = 1;
            }else{
                // More than one pokemon attended the battle and they all fled.
                // We think our pokemon is strong enough to defeat all the wild pokemon in the battle, so it is not possible
                // that the battle ends because we lost.
                num_removed_pokemon = num_initial_sprites;
            }
            break;
        }

        // now in battle menu
        battle_menu_detectd = true;
        const auto sprite_detection_frame = env.console.video().snapshot().frame;
        const auto sprites_remain = battle_sprite_watcher.detect_sprites(*sprite_detection_frame);
        size_t num_sprites_remain = 0;
        for(bool remain : sprites_remain){
            num_sprites_remain += remain;
        }
        num_removed_pokemon = num_initial_sprites - num_sprites_remain;

        if (PreloadSettings::instance().DEVELOPER_MODE){
            dump_debug_image(env.logger(), "PokemonLA/AutoMultiSpawn", "battle_sprite_" + std::to_string(num_sprites_remain), *sprite_detection_frame);
        }
        env.log("Found battle menu, num sprites removed: " + std::to_string(num_removed_pokemon));
        
        if (num_removed_pokemon > num_to_despawn){
            // Oh no, we removed more than needed.
            // XXX can try to reset the game to fix this. But for now let user handles this.
            env.log("Removed more than needed!");
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Removed more pokemon than needed!",
                env.console
            );
        }else if (num_removed_pokemon < num_to_despawn){

            // Press A to select moves
            pbf_press_button(context, BUTTON_A, 10, 100);
            context.wait_for_all_requests();
            use_next_move_with_pp(env.console, context, 0, cur_move);
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
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Cannot detect end of battle when escaping.",
                env.console
            );
        }
    }

    goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Mirelands_Mirelands);
    return num_removed_pokemon;
}

PokemonDetails AutoMultiSpawn::go_to_spawn_point_and_try_focusing_pokemon(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    size_t nun_pokemon_left
){
    // From camp fly to the spawn point, focus on a target pokemon and start a battle
    change_mount(env.console, context, MountState::BRAVIARY_ON);
    pbf_wait(context, 40);
    
    // Move to spawn location on Braviary
    pbf_move_left_joystick(context, 255, 165, 150, 0); // 170
    pbf_press_button(context, BUTTON_B, 200, 10);
    pbf_mash_button(context, BUTTON_B, 1500); // 1450

    // Descend down from the air:
    for(int i = 0; i < 2 ; i++){
        change_mount(env.console, context, MountState::BRAVIARY_OFF);
        context.wait_for(std::chrono::milliseconds(300));
        change_mount(env.console, context, MountState::BRAVIARY_ON);
    }

    // pbf_press_button(context, BUTTON_PLUS, 20, 150); // jump down from Braviary
    // for(int i = 0; i < 2; i++){
    //     pbf_press_button(context, BUTTON_PLUS, 20, 50); // Call back Braviary to stop falling
    //     pbf_press_button(context, BUTTON_PLUS, 20, 150); // fall down again
    // }
    // In case the character hits a tree and change the Braviary mount state due to the hit,
    // use visual feedback to makse sure the character is now dismounted.
    change_mount(env.console, context, MountState::BRAVIARY_OFF);
    pbf_wait(context, 50);
    
    // Move forward on foot
    pbf_move_left_joystick(context, 128, 0, 160, 0);

    context.wait_for_all_requests();

    // Try three focus sessions:
    const int max_focus_try = 5;
    for(int i = 0; i < max_focus_try; i++){
        // ret.first: whether we can focus on some pokemon
        // ret.second: the details of the target pokemon being focused, or empty if no target pokemon found.
        MultiSpawn spawn = SPAWN;
        const auto ret = control_focus_to_throw(
            env, context,
            TARGET_POKEMON[(size_t)spawn],
            WILD_NEARBY_POKEMON[(size_t)spawn],
            LANGUAGE
        );
        if (ret.first){
            return ret.second;
        }

        if (i + 1 < max_focus_try){
            env.log("Try another focus attempt.");
            pbf_wait(context, 200);
            context.wait_for_all_requests();
        }

        if (i == 2 && nun_pokemon_left == 1){
            // If its' only one pokemon to despawn, then we don't need to worry about scare multiple pokemon at once.
            // We can move closer.
            pbf_move_left_joystick(context, 128, 0, 150, 60);
            context.wait_for_all_requests();
        }
    }

    return PokemonDetails();
}


}
}
}
