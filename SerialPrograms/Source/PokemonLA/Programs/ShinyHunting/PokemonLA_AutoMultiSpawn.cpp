/*  Auto Multi-Spawn
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA_AutoMultiSpawn.h"
#include "PokemonLA/Inference/PokemonLA_WildPokemonFocusDetector.h"

#include <iostream>
#include <sstream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Pokemon;

const int MAX_DESPAWN_COUNT[] = {
    2 // Mirelands - Hippopotas
};

enum class TimeOfDay{
    MORNING,
    MIDDAY,
    EVENING,
    MIDNIGHT
};


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
    : SPAWN("<b>Spawn Point</b>:",
    {
        "Mirelands - Hippopotas"
    }, 0)
    , PATH(false, "<b>Multi-Spawn Path<b>:<br>e.g. \"A1|A1|A2|A2|A1|A1|A1|A2\".", "", "")
    , NOTIFICATIONS({
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
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

    {
        std::ostringstream os;
        for(size_t i = 0; i < path_despawns.size(); i++){
            if (i >0){
                os << '|';
            }
            // TODO: if murkrow path is added, need to output 'N'
            os << 'A' << path_despawns[i] << '(' << (i % 2 == 0 ? 'M' : 'D') << ')';
        }
        env.log("The path is: " + os.str());
    }

    return path_despawns;
}


void AutoMultiSpawn::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    const int max_num_despawn = MAX_DESPAWN_COUNT[SPAWN];

    // Parse input path string:
    std::vector<int> path_despawns = parse_multispawn_path(env, PATH.get(), max_num_despawn);
    
    // change_time_of_day(TimeOfDay.MORNING);

    for(size_t i = 0; i < path_despawns.size(); i++){
#if 1
        auto go_to_spawn_point_and_focus = [&]() -> PokemonDetails {
            goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Mirelands_Mirelands);
            change_mount(env.console, context, MountState::BRAVIARY_ON);
            pbf_wait(context, 40);
            
            // Move to spawn location on Braviary
            pbf_move_left_joystick(context, 255, 165, 150, 0); // 170
            pbf_press_button(context, BUTTON_B, 200, 10);
            pbf_mash_button(context, BUTTON_B, 1500); // 1450

            // Descend down from the air:
            pbf_press_button(context, BUTTON_PLUS, 20, 100); // jump down from Braviary
            for(int i = 0; i < 3; i++){
                pbf_press_button(context, BUTTON_PLUS, 20, 50); // Call back Braviary to stop falling
                pbf_press_button(context, BUTTON_PLUS, 20, 100); // fall down again
            }
            
            // Move forward on foot
            pbf_move_left_joystick(context, 128, 0, 150, 0);

            context.wait_for_all_requests();

            // Focus on one pokemon and read pokemon info:
            WildPokemonFocusDetector focus_detector(env.console.logger(), env.console);
            int ret = run_until(
            env.console, context,
                [&](BotBaseContext& context){
                    pbf_press_button(context, BUTTON_ZL, 400, 10);
                    context.wait_for_all_requests();
                },
                {{focus_detector}}
            );
            
            if (ret == 0){
                env.log("Found pokemon focus tab.");
                const auto details = focus_detector.get_focus_info();
                if (details.first.name_candidates.size() > 0){
                    env.log("Found pokemon " + *details.first.name_candidates.begin());

                    return details.first;
                }
            }

            return PokemonDetails();
        };

        // Try to go to spawn point and focus on one pokemon
        PokemonDetails focused_pokemon;
        const size_t num_tries = 5;
        for(size_t j = 0; j < num_tries; j++){
            focused_pokemon = go_to_spawn_point_and_focus();
            if (focused_pokemon.name_candidates.size() > 0){
                // We found one
                break;
            }
            env.log("Cannot focus on any pokemon. Retry.");
            goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Mirelands_Mirelands);
            // TODO: May need to reset time of day here
        }

        if (focused_pokemon.name_candidates.size() == 0){
            throw  OperationFailedException(env.console, "Cannot focus on a pokemon after going to the spawn point  " + std::to_string(num_tries) + " times");
        }

        // XXX
        cout << "End of one step in the path" << endl;
//        break;
#endif
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}




}
}
}
