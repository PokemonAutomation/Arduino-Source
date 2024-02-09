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
#include "PokemonSV/Inference/PokemonSV_BlueberryQuestDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV_BlueberryQuests.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

void return_to_plaza(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context) {
    //Modified version of handle_battles_and_back_to_pokecenter()
    bool returned_to_pokecenter = false;

    while(!returned_to_pokecenter){
        EncounterWatcher encounter_watcher(console, COLOR_RED);
        int ret = run_until(
            console, context,
            [&](BotBaseContext& context){
                //Exit any dialogs (ex. Cyrano upgrading BBQs)
                OverworldWatcher overworld(COLOR_RED);
                int ret_overworld = run_until(
                    console, context,
                    [&](BotBaseContext& context) {
                        pbf_mash_button(context, BUTTON_B, 10000);
                    },
                    { overworld }
                    );
                if (ret_overworld == 0) {
                    console.log("Overworld detected.");
                }
                context.wait_for_all_requests();

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
                //Arena trap and no smoke ball. Attack?
                //GO_HOME_WHEN_DONE.run_end_of_program(context);
                //throw;
            }
        }
    }
}

void read_quests(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS, std::vector<std::string>& quest_list) {
    //Open quest list. Wait for it to open.
    WhiteButtonWatcher right_panel(COLOR_BLUE, WhiteButton::ButtonB, {0.484, 0.117, 0.022, 0.037});
    int result = run_until(
        console, context,
        [&](BotBaseContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 50, 20);
            pbf_wait(context, 200);
            context.wait_for_all_requests();
        },
        {{ right_panel }}
    );
    if (result == 0){
        console.log("Found quest panel.");
    }
    context.wait_for_all_requests();

    //Read in the initial 4 quests.
    VideoSnapshot screen = console.video().snapshot();
    BlueberryQuestDetector first_quest_detector(console.logger(), COLOR_GREEN, BBQ_OPTIONS.LANGUAGE, BlueberryQuestDetector::QuestPosition::FIRST);
    BlueberryQuestDetector second_quest_detector(console.logger(), COLOR_GREEN, BBQ_OPTIONS.LANGUAGE, BlueberryQuestDetector::QuestPosition::SECOND);
    BlueberryQuestDetector third_quest_detector(console.logger(), COLOR_GREEN, BBQ_OPTIONS.LANGUAGE, BlueberryQuestDetector::QuestPosition::THIRD);
    BlueberryQuestDetector fourth_quest_detector(console.logger(), COLOR_GREEN, BBQ_OPTIONS.LANGUAGE, BlueberryQuestDetector::QuestPosition::FOURTH);

    quest_list.push_back(first_quest_detector.detect_quest(screen));
    quest_list.push_back(second_quest_detector.detect_quest(screen));
    quest_list.push_back(third_quest_detector.detect_quest(screen));
    
    std::string fourth_quest = fourth_quest_detector.detect_quest(screen);
    if (fourth_quest != "") {
        quest_list.push_back(fourth_quest_detector.detect_quest(screen));
    }

    //Close quest list
    pbf_mash_button(context, BUTTON_B, 100);
    context.wait_for_all_requests();
}

void process_quest_list(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS, std::vector<std::string>& quest_list, std::vector<std::string>& quests_to_do, int& eggs_hatched) {
    //Put all do-able quests into a different list
    for (auto n : quest_list) {
        if (not_possible_quests.contains(n)) {
            console.log("Quest not possible: " + n);
        }
        else {
            //Check eggs remaining
            if (n == "hatch-egg" && BBQ_OPTIONS.NUM_EGGS <= eggs_hatched) {
                console.log("Out of eggs! Quest not possible.");

            }
            else {
                console.log("Quest possible: " + n);
                quests_to_do.push_back(n);
            }
        }
    }
    //Check that quests_to_do is not empty (after completing all quests on the list, be sure to erase it.
    //Lag might be a problem in multi - look into making slots like menu-left navigation
    if (quests_to_do.size() == 0) {
        console.log("No possible quests! Rerolling all quests.");
        WhiteButtonWatcher right_panel(COLOR_BLUE, WhiteButton::ButtonB, {0.484, 0.117, 0.022, 0.037});
        int result = run_until(
            console, context,
            [&](BotBaseContext& context){
                pbf_press_dpad(context, DPAD_RIGHT, 50, 20);
                pbf_wait(context, 200);
                context.wait_for_all_requests();
            },
            {{ right_panel }}
        );
        if (result == 0){
            console.log("Found quest panel.");
        }
        context.wait_for_all_requests();

        //Reroll all.
        for (int i = 0; i < quest_list.size(); i++) {
            pbf_press_button(context, BUTTON_A, 20, 50);
            pbf_press_button(context, BUTTON_A, 20, 50); //Yes.
            pbf_wait(context, 100);
            context.wait_for_all_requests();
            pbf_press_dpad(context, DPAD_DOWN, 20, 20);
            pbf_wait(context, 100);
            context.wait_for_all_requests();
        }

        //Close quest panel
        pbf_mash_button(context, BUTTON_B, 100);
        context.wait_for_all_requests();
    }

}


}
}
}
