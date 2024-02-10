/*  Connect to Integer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
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

const std::map<std::string, BBQuests>& BBQuests_TOKEN_TO_ENUM() {
    static std::map<std::string, BBQuests> data{
        {"auto-10",             BBQuests::auto_10},
        {"make-tm",             BBQuests::make_tm },
        {"pickup-10",           BBQuests::pickup_10},
        {"sneak-up",            BBQuests::sneak_up},
        {"photo-fly",           BBQuests::photo_fly},
        {"photo-swim",          BBQuests::photo_swim},
        {"photo-canyon",        BBQuests::photo_canyon},
        {"photo-coastal",       BBQuests::photo_coastal},
        {"photo-polar",         BBQuests::photo_polar},
        {"photo-savanna",       BBQuests::photo_savanna},
        {"tera-self-defeat",    BBQuests::tera_self_defeat},
        {"travel-500",          BBQuests::travel_500},
        {"catch-any",           BBQuests::catch_any},
        {"catch-normal",        BBQuests::catch_normal},
        {"catch-fighting",      BBQuests::catch_fighting},
        {"catch-flying",        BBQuests::catch_flying},
        {"catch-poison",        BBQuests::catch_poison},
        {"catch-ground",        BBQuests::catch_ground},
        {"catch-rock",          BBQuests::catch_rock},
        {"catch-bug",           BBQuests::catch_bug},
        {"catch-ghost",         BBQuests::catch_ghost},
        {"catch-steel",         BBQuests::catch_steel},
        {"catch-fire",          BBQuests::catch_fire},
        {"catch-water",         BBQuests::catch_water},
        {"catch-grass",         BBQuests::catch_grass},
        {"catch-electric",      BBQuests::catch_electric},
        {"catch-psychic",       BBQuests::catch_psychic},
        {"catch-ice",           BBQuests::catch_ice},
        {"catch-dragon",        BBQuests::catch_dragon},
        {"catch-dark",          BBQuests::catch_dark},
        {"catch-fairy",         BBQuests::catch_fairy},
        {"wash-pokemon",        BBQuests::wash_pokemon},
        {"wild-tera",           BBQuests::wild_tera},
        {"auto-30",             BBQuests::auto_30},
        {"tera-raid",           BBQuests::tera_raid},
        {"sandwich-three",      BBQuests::sandwich_three},
        {"bitter-sandwich",     BBQuests::bitter_sandwich},
        {"sweet-sandwich",      BBQuests::sweet_sandwich},
        {"salty-sandwich",      BBQuests::salty_sandwich},
        {"sour-sandwich",       BBQuests::sour_sandwich},
        {"spicy-sandwich",      BBQuests::spicy_sandwich},
        {"hatch-egg",           BBQuests::hatch_egg},
        {"photo-normal",        BBQuests::photo_normal},
        {"photo-fighting",      BBQuests::photo_fighting},
        {"photo-flying",        BBQuests::photo_flying},
        {"photo-poison",        BBQuests::photo_poison},
        {"photo-ground",        BBQuests::photo_ground},
        {"photo-rock",          BBQuests::photo_rock},
        {"photo-bug",           BBQuests::photo_bug},
        {"photo-ghost",         BBQuests::photo_ghost},
        {"photo-steel",         BBQuests::photo_steel},
        {"photo-fire",          BBQuests::photo_fire},
        {"photo-water",         BBQuests::photo_water},
        {"photo-grass",         BBQuests::photo_grass},
        {"photo-electric",      BBQuests::photo_electric},
        {"photo-psychic",       BBQuests::photo_psychic},
        {"photo-ice",           BBQuests::photo_ice},
        {"photo-dragon",        BBQuests::photo_dragon},
        {"photo-dark",          BBQuests::photo_dark},
        {"photo-fairy",         BBQuests::photo_fairy},
        {"ditto-central",       BBQuests::ditto_central},
        {"ditto-canyon",        BBQuests::ditto_canyon},
        {"ditto-coastal",       BBQuests::ditto_coastal},
        {"ditto-polar",         BBQuests::ditto_polar},
        {"ditto-savanna",       BBQuests::ditto_savanna},
        {"group-canyon",        BBQuests::group_canyon},
        {"group-coastal",       BBQuests::group_coastal},
        {"group-polar",         BBQuests::group_polar},
        {"group-savanna",       BBQuests::group_savanna},
        {"group-eyewear",       BBQuests::group_eyewear},
        {"group-nonuniform",    BBQuests::group_nonuniform},
        {"group-masks",         BBQuests::group_masks},
        {"sandwich-four",       BBQuests::sandwich_four},
        {"catch-hint",          BBQuests::catch_hint},
        {"catch-hint2",         BBQuests::catch_hint2},
    };
    return data;
}

BBQuests BBQuests_string_to_enum(const std::string& token){
    auto iter = BBQuests_TOKEN_TO_ENUM().find(token);
    if (iter == BBQuests_TOKEN_TO_ENUM().end()){
        return BBQuests::UnableToDetect;
    }
    return iter->second;
}


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

int read_BP(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context) {
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
    
    VideoSnapshot screen = console.video().snapshot();
    ImageRGB32 BP_value = to_blackwhite_rgb32_range(
        extract_box_reference(screen, ImageFloatBox(0.866, 0.019, 0.091, 0.041)),
        combine_rgb(198, 198, 198), combine_rgb(255, 255, 255), true
    );

    //Close panel
    pbf_mash_button(context, BUTTON_B, 100);
    context.wait_for_all_requests();

    return OCR::read_number(console, BP_value);
}

void read_quests(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS, std::vector<BBQuests>& quest_list) {
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

    quest_list.push_back(BBQuests_string_to_enum(first_quest_detector.detect_quest(screen)));
    quest_list.push_back(BBQuests_string_to_enum(second_quest_detector.detect_quest(screen)));
    quest_list.push_back(BBQuests_string_to_enum(third_quest_detector.detect_quest(screen)));
    
    std::string fourth_quest = fourth_quest_detector.detect_quest(screen);
    if (fourth_quest != "") {
        quest_list.push_back(BBQuests_string_to_enum(fourth_quest_detector.detect_quest(screen)));
    }

    //Close quest list
    pbf_mash_button(context, BUTTON_B, 100);
    context.wait_for_all_requests();
}

void process_quest_list(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS, std::vector<BBQuests>& quest_list, std::vector<BBQuests>& quests_to_do, int& eggs_hatched) {
    //Put all do-able quests into a different list
    for (auto n : quest_list) {
        if (not_possible_quests.contains(n)) {
            console.log("Quest not possible");
        }
        else {
            //Check eggs remaining
            if (n == BBQuests::hatch_egg && BBQ_OPTIONS.NUM_EGGS <= eggs_hatched) {
                console.log("Out of eggs! Quest not possible.");

            }
            else {
                console.log("Quest possible");
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

void process_and_do_quest(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS, BBQuests& current_quest, int& eggs_hatched) {
    //bool quest_completed = false;
    
    //while(!quest_completed)

    /*switch (current_quest) {
    case "photo-swim":

        break;
    default:
        //throw error this should not be possible
        break;
    }
    */




    //Validate quest was completed by checking list

}

}
}
}
