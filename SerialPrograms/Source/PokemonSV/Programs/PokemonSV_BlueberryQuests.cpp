/*  Connect to Integer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV_BlueberryQuests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

void return_to_plaza(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context) {
    //Modified version of handle_battles_and_back_to_pokecenter()
    //TODO: Attempt to handle Cyrano upgrading quests - no way for me to test this atm.
    bool returned_to_pokecenter = false;

    while(!returned_to_pokecenter){
        EncounterWatcher encounter_watcher(console, COLOR_RED);
        int ret = run_until(
            console, context,
            [&](BotBaseContext& context){
                open_map_from_overworld(info, console, context);

                //Move cursor to top left corner - even works when at Entrance fly point
                pbf_press_button(context, BUTTON_ZL, 40, 100);
                pbf_move_left_joystick(context, 0, 0, 500, 40);

                //Now move toward center
                pbf_move_left_joystick(context, 255, 255, 250, 40);
                pbf_press_button(context, BUTTON_ZR, 40, 100);

                //The only pokecenter on the map is Central Plaza
                fly_to_closest_pokecenter_on_map(info, console, context);

                context.wait_for_all_requests();
                returned_to_pokecenter = true;
            },
            {
                static_cast<VisualInferenceCallback&>(encounter_watcher),
                static_cast<AudioInferenceCallback&>(encounter_watcher),
            }
        );
        encounter_watcher.throw_if_no_sound();
        if (ret >= 0){
            console.log("Detected battle. Running from battle.");
            try{
                //TODO: Currently Smoke Ball or Flying type required due to Arena Trap
                NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
                battle_menu.move_to_slot(console, context, 3);
                pbf_press_button(context, BUTTON_A, 10, 50);
            }catch (...){
                //GO_HOME_WHEN_DONE.run_end_of_program(context);
                //throw;
            }
        }
    }
}



}
}
}
