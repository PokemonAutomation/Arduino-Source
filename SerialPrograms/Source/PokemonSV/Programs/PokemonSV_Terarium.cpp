/*  Terarium
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV_Terarium.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

void return_to_plaza(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    stream.log("Attempting to return to Central Plaza.");
    //Modified version of handle_battles_and_back_to_pokecenter()
    bool returned_to_pokecenter = false;
    bool laggy = false;

    while(!returned_to_pokecenter){
        EncounterWatcher encounter_watcher(stream, COLOR_RED);
        int ret = run_until<ProControllerContext>(
            stream, context,
            [&](ProControllerContext& context){
                //Exit any dialogs (ex. Cyrano upgrading BBQs)
                OverworldWatcher overworld(stream.logger(), COLOR_RED);
                int ret_overworld = run_until<ProControllerContext>(
                    stream, context,
                    [&](ProControllerContext& context){
                        pbf_mash_button(context, BUTTON_B, 10000);
                    },
                    {overworld}
                );
                if (ret_overworld == 0){
                    stream.log("Overworld detected.");
                }
                context.wait_for_all_requests();

                try{
                    open_map_from_overworld(info, stream, context);
                }catch (...){
                    jump_off_wall_until_map_open(info, stream, context);
                }

                //Move cursor to top left corner - even works when at Entrance fly point
                pbf_press_button(context, BUTTON_ZL, 40, 100);
                pbf_move_left_joystick(context, 0, 0, 500, 40);

                //Now move toward center
                if (laggy){
                    pbf_move_left_joystick(context, 255, 255, 300, 40); //overshoot by a bit (still works even if not laggy)
                }else{
                    pbf_move_left_joystick(context, 255, 255, 250, 40); //250 is more accurate but 300 helps with lag
                }
                pbf_press_button(context, BUTTON_ZR, 40, 100);

                try{
                    //The only pokecenter on the map is Central Plaza
                    fly_to_closest_pokecenter_on_map(info, stream, context);
                    context.wait_for_all_requests();
                    returned_to_pokecenter = true;
                }catch(...){
                    stream.log("Failed to return to Pokecenter. Closing map and retrying.");
                    laggy = true;
                }
                context.wait_for_all_requests();
            },
            {
                static_cast<VisualInferenceCallback&>(encounter_watcher),
                static_cast<AudioInferenceCallback&>(encounter_watcher),
            }
        );
        if (ret == 0){
            stream.log("Battle menu detected.");
            encounter_watcher.throw_if_no_sound();

            bool is_shiny = (bool)encounter_watcher.shiny_screenshot();
            if (is_shiny){
                stream.log("Shiny detected!");
                pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
                throw ProgramFinishedException();
            }else{
                stream.log("Detected battle. Running from battle.");
                try{
                    //Smoke Ball or Flying type required due to Arena Trap
                    NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
                    battle_menu.move_to_slot(stream, context, 3);
                    pbf_press_button(context, BUTTON_A, 10, 50);
                }catch (...){
                    stream.log("Unable to flee.");
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Unable to flee!",
                        stream
                    );
                }
            }
        }
    }
    context.wait_for_all_requests();
}

void map_move_cursor_fly(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    uint8_t x, uint8_t y,
    uint8_t hold, uint8_t release,
    std::string location
){
    stream.log("Attempting to fly to " + location + ".");

    for (int i = 0; i < 3; i++){
        try{
            open_map_from_overworld(info, stream, context);
            pbf_move_left_joystick(context, x, y, hold, release);
            pbf_press_button(context, BUTTON_ZL, 40, 100);
            fly_to_overworld_from_map(info, stream, context);
            break;
        }
        catch(...){
            stream.log("Failed to fly! Closing map and retrying.");
            press_Bs_to_back_to_overworld(info, stream, context);
            if (i == 2){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Unable to fly to " + location + "!",
                    stream
                );
            }
        }
    }
}

void central_to_polar_rest(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    map_move_cursor_fly(info, stream, context, 75, 0, 230, 20, "Polar Rest Area");
}

void central_to_polar_class1(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    map_move_cursor_fly(info, stream, context, 0, 20, 150, 20, "Polar Classroom 1");
}

void central_to_polar_plaza(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    map_move_cursor_fly(info, stream, context, 20, 25, 245, 20, "Polar Plaza");
}

void central_to_coastal_plaza(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    map_move_cursor_fly(info, stream, context, 180, 0, 210, 20, "Coastal Plaza");
}

void central_to_canyon_plaza(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    map_move_cursor_fly(info, stream, context, 0, 255, 215, 20, "Canyon Plaza");
}

void central_to_savanna_plaza(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    map_move_cursor_fly(info, stream, context, 165, 255, 180, 20, "Savanna Plaza");
}

void central_to_canyon_rest(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    map_move_cursor_fly(info, stream, context, 0, 140, 160, 20, "Canyon Rest Area");
}

void central_to_savanna_class(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    map_move_cursor_fly(info, stream, context, 255, 220, 140, 20, "Savanna Classroom");
}

void central_to_chargestone(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context){
    map_move_cursor_fly(info, stream, context, 0, 135, 130, 20, "Chargestone Cavern");
}

void jump_glide_fly(
    VideoStream& stream, ProControllerContext& context,
    bool inverted_flight,
    uint16_t hold_up,
    uint16_t flight_wait,
    uint16_t drop_time
){
    stream.log("Jump, glide, fly.");

    ssf_press_button(context, BUTTON_B, 0ms, 800ms);
    ssf_press_button(context, BUTTON_B, 0ms, 160ms, 80ms);
    ssf_press_button(context, BUTTON_B, 0ms, 160ms);
    pbf_wait(context, 100);
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_LCLICK, 50, 0);


    if (inverted_flight){
        pbf_move_left_joystick(context, 128, 255, hold_up, 250);
    }else{
        pbf_move_left_joystick(context, 128, 0, hold_up, 250);
    }

    pbf_wait(context, flight_wait);
    context.wait_for_all_requests();

    pbf_press_button(context, BUTTON_B, 20, 50);
    pbf_wait(context, drop_time);
    context.wait_for_all_requests();
}

}
}
}
