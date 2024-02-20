/*  Blueberry Quests
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
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/Inference/PokemonSV_BlueberryQuestDetector.h"
#include "PokemonSV/Programs/Battles/PokemonSV_BasicCatcher.h"
#include "PokemonSV/Programs/Eggs/PokemonSV_EggRoutines.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRelease.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxEggDetector.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV/Programs/Farming/PokemonSV_BlueberryCatchPhoto.h"
#include "PokemonSV_BlueberryQuests.h"

#include<vector>

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
        {"",                    BBQuests::UnableToDetect},
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
    console.log("Return to plaza");
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

                try {
                    //The only pokecenter on the map is Central Plaza
                    fly_to_closest_pokecenter_on_map(info, console, context);
                    context.wait_for_all_requests();
                    returned_to_pokecenter = true;
                }
                catch(...) {
                    console.log("Failed to return to Pokecenter. Closing map and retrying.");
                }
                context.wait_for_all_requests();
            },
            {
                static_cast<VisualInferenceCallback&>(encounter_watcher),
                static_cast<AudioInferenceCallback&>(encounter_watcher),
            }
        );
        if (ret >= 0) {
            console.log("Battle menu detected.");
            encounter_watcher.throw_if_no_sound();

            bool is_shiny = (bool)encounter_watcher.shiny_screenshot();
            if (is_shiny) {
                console.log("Shiny detected!");
                pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
                throw ProgramFinishedException();
            }
            else {
                console.log("Detected battle. Running from battle.");
                try{
                    //Smoke Ball or Flying type required due to Arena Trap
                    NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
                    battle_menu.move_to_slot(console, context, 3);
                    pbf_press_button(context, BUTTON_A, 10, 50);
                }catch (...){
                    console.log("Unable to flee.");
                    throw OperationFailedException(
                        ErrorReport::SEND_ERROR_REPORT, console,
                        "Unable to flee!",
                        true
                    );
                }
            }
        }
    }
    context.wait_for_all_requests();
}

int read_BP(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context) {
    WhiteButtonWatcher right_panel(COLOR_BLUE, WhiteButton::ButtonB, {0.484, 0.117, 0.022, 0.037});
    int result = run_until(
        console, context,
        [&](BotBaseContext& context){
            for (int i = 0; i < 6; i++) { //try 6 times
                pbf_press_dpad(context, DPAD_RIGHT, 50, 20);
                pbf_wait(context, 200);
                context.wait_for_all_requests();
            }
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

std::vector<BBQuests> read_quests(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS) {
    std::vector<BBQuests> quest_list;

    //Open quest list. Wait for it to open.
    WhiteButtonWatcher right_panel(COLOR_BLUE, WhiteButton::ButtonB, {0.484, 0.117, 0.022, 0.037});
    int result = run_until(
        console, context,
        [&](BotBaseContext& context){
            for (int i = 0; i < 6; i++) { //try 6 times
                pbf_press_dpad(context, DPAD_RIGHT, 50, 20);
                pbf_wait(context, 200);
                context.wait_for_all_requests();
            }
        },
        {{ right_panel }}
    );
    if (result == 0){
        console.log("Found quest panel.");
    }
    context.wait_for_all_requests();

    //Read in the initial 4 quests.
    console.log("Reading quests.");
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
        quest_list.push_back(BBQuests_string_to_enum(fourth_quest));
    }

    //Close quest list
    pbf_mash_button(context, BUTTON_B, 100);
    context.wait_for_all_requests();

    return quest_list;
}

std::vector<BBQuests> process_quest_list(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS, std::vector<BBQuests>& quest_list, uint8_t& eggs_hatched) {
    std::vector<BBQuests> quests_to_do;

    console.log("Processing quests.");
    //Put all do-able quests into a different list
    for (auto n : quest_list) {
        if (not_possible_quests.contains(n)) {
            console.log("Quest not possible");
        }
        else {
            //Check eggs remaining
            if (n == BBQuests::hatch_egg && eggs_hatched < BBQ_OPTIONS.NUM_EGGS) {
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
        //TODO: This does not handle out of BP.
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

    return quests_to_do;
}

bool process_and_do_quest(const ProgramInfo& info, AsyncDispatcher& dispatcher, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS, BBQuests& current_quest, uint8_t& eggs_hatched) {
    bool quest_completed = false;
    int quest_attempts = 0;

    while (!quest_completed) {
        switch (current_quest) {
        case BBQuests::make_tm:
            quest_make_tm(info, console, context);
            break;
        case BBQuests::travel_500:
            quest_travel_500(info, console, context);
            break;
        case BBQuests::tera_self_defeat:
            quest_tera_self_defeat(info, console, context, BBQ_OPTIONS);
            break;
        case BBQuests::sneak_up:
            quest_sneak_up(info, console, context, BBQ_OPTIONS);
            break;
        case BBQuests::wild_tera:
            quest_wild_tera(info, console, context, BBQ_OPTIONS);
            break;
        case BBQuests::wash_pokemon:
            quest_wash_pokemon(info, console, context);
            break;
        case BBQuests::hatch_egg:
            quest_hatch_egg(info, console, context, BBQ_OPTIONS);
            break;
        case BBQuests::bitter_sandwich: case BBQuests::salty_sandwich: case BBQuests::sour_sandwich: case BBQuests::spicy_sandwich: case BBQuests::sweet_sandwich: case BBQuests::sandwich_three:
            quest_sandwich(info, dispatcher, console, context, BBQ_OPTIONS, current_quest);
            break;
        //All involve taking pictures
        case BBQuests::photo_fly: case BBQuests::photo_swim: case BBQuests::photo_canyon: case BBQuests::photo_coastal: case BBQuests::photo_polar: case BBQuests::photo_savanna:
        case BBQuests::photo_normal: case BBQuests::photo_fighting: case BBQuests::photo_flying: case BBQuests::photo_poison: case BBQuests::photo_ground:
        case BBQuests::photo_rock: case BBQuests::photo_bug: case BBQuests::photo_ghost: case BBQuests::photo_steel: case BBQuests::photo_fire: case BBQuests::photo_water:case BBQuests::photo_grass:
        case BBQuests::photo_electric: case BBQuests::photo_psychic: case BBQuests::photo_ice: case BBQuests::photo_dragon: case BBQuests::photo_dark: case BBQuests::photo_fairy:
            quest_photo(info, console, context, BBQ_OPTIONS, current_quest);
            break;
        //All involve catching a pokemon
        case BBQuests::catch_any: case BBQuests::catch_normal: case BBQuests::catch_fighting: case BBQuests::catch_flying: case BBQuests::catch_poison: case BBQuests::catch_ground:
        case BBQuests::catch_rock: case BBQuests::catch_bug: case BBQuests::catch_ghost: case BBQuests::catch_steel: case BBQuests::catch_fire: case BBQuests::catch_water:case BBQuests::catch_grass:
        case BBQuests::catch_electric: case BBQuests::catch_psychic: case BBQuests::catch_ice: case BBQuests::catch_dragon: case BBQuests::catch_dark: case BBQuests::catch_fairy:
            quest_catch(info, console, context, BBQ_OPTIONS, current_quest);
            break;
        }

        //Validate quest was completed by checking list
        std::vector<BBQuests> quest_list = read_quests(info, console, context, BBQ_OPTIONS);
        if (std::find(quest_list.begin(), quest_list.end(), current_quest) != quest_list.end()) {
            console.log("Current quest exists on list. Quest did not complete.");
            quest_attempts++;
        }
        else {
            console.log("Current quest was not found. Quest completed!");
            if (current_quest == BBQuests::hatch_egg) {
                eggs_hatched++;
                console.log("Eggs hatched: " + std::to_string(eggs_hatched));
            }

            quest_completed = true;
        }

        if (quest_attempts > BBQ_OPTIONS.NUM_RETRIES) {
            console.log("Failed to complete a quest multiple times. Skipping.", COLOR_RED);
            break;
        }
    }
    return quest_completed;
}

void quest_make_tm(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context) {
    console.log("Quest: Make TM");

    //Mount and then dismount in case you're crouched
    pbf_press_button(context, BUTTON_PLUS, 20, 105);
    pbf_press_button(context, BUTTON_PLUS, 20, 105);
    context.wait_for_all_requests();

    PromptDialogWatcher makeTM(COLOR_RED);
    OverworldWatcher overworld(COLOR_BLUE);

    pbf_move_left_joystick(context, 255, 0, 100, 20);
    pbf_press_button(context, BUTTON_L, 10, 50);

    //The plaza is extemely bright and tends to throw off the detectors
    pbf_press_button(context, BUTTON_A, 20, 200);
    pbf_wait(context, 200);
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 20, 200);
    pbf_wait(context, 200);
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 20, 200);
    pbf_wait(context, 200);
    context.wait_for_all_requests();

    int make_tm = run_until(
        console, context,
        [&](BotBaseContext& context){
            for (int i = 0; i < 229; i++) { //229 is max number of TMs
                //click on tm
                pbf_press_button(context, BUTTON_A, 20, 50);
                pbf_wait(context, 100);
                context.wait_for_all_requests();

                //not craftable, close and move on to next
                pbf_press_button(context, BUTTON_A, 20, 50);
                pbf_press_dpad(context, DPAD_RIGHT, 20, 20);
                pbf_wait(context, 100);
                context.wait_for_all_requests();
            }
        },
        {{ makeTM }}
    );
    if (make_tm == 0){
        console.log("Craftable TM found.");
    }
    else {
        console.log("Failed to find craftable TM!");
    }
    context.wait_for_all_requests();

    //Make TM
    pbf_mash_button(context, BUTTON_A, 220);
    context.wait_for_all_requests();

    int exit = run_until(
        console, context,
        [&](BotBaseContext& context){
            //click on tm
            pbf_mash_button(context, BUTTON_B, 2000);
        },
        {{ overworld }}
    );
    if (exit == 0){
        console.log("Overworld detected.");
    }
    context.wait_for_all_requests();
    
    open_map_from_overworld(info, console, context);
    pbf_press_button(context, BUTTON_ZL, 40, 100);
    fly_to_overworld_from_map(info, console, context);
    context.wait_for_all_requests();
}

void quest_travel_500(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context) {
    console.log("Quest: Travel 500 meters.");

    //Mount and then dismount in case you're crouched
    pbf_press_button(context, BUTTON_PLUS, 20, 105);
    pbf_press_button(context, BUTTON_PLUS, 20, 105);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, 0, 0, 100, 20);
    pbf_move_left_joystick(context, 128, 0, 150, 20);
    pbf_move_left_joystick(context, 0, 128, 140, 20);

    pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

    for(int j = 0; j < 60; j++){ //One min just to be safe.
        pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE,
            128, 0, 255, 128, TICKS_PER_SECOND);
    }
    context.wait_for_all_requests();

    open_map_from_overworld(info, console, context);
    pbf_press_button(context, BUTTON_ZL, 40, 100);
    fly_to_overworld_from_map(info, console, context);
    context.wait_for_all_requests();
}

void quest_tera_self_defeat(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS) {
    EncounterWatcher encounter_watcher(console, COLOR_RED);
    console.log("Quest: Tera-self and defeat any wild.");
    //Navigate to target and start battle
    int ret = run_until(
        console, context,
        [&](BotBaseContext& context) {

            open_map_from_overworld(info, console, context);
            pbf_move_left_joystick(context, 0, 255, 215, 20);
            pbf_press_button(context, BUTTON_ZL, 40, 100);
            fly_to_overworld_from_map(info, console, context);

            pbf_move_left_joystick(context, 205, 64, 20, 105);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            //Jump, glide, fly
            ssf_press_button(context, BUTTON_B, 0, 100);
            ssf_press_button(context, BUTTON_B, 0, 20, 10);
            ssf_press_button(context, BUTTON_B, 0, 20);
            pbf_wait(context, 100);
            context.wait_for_all_requests();
            pbf_press_button(context, BUTTON_LCLICK, 50, 0);

            if (BBQ_OPTIONS.INVERTED_FLIGHT) {
                pbf_move_left_joystick(context, 128, 255, 1000, 250);
            }
            else {
                pbf_move_left_joystick(context, 128, 0, 1000, 250);
            }

            pbf_wait(context, 1650);
            context.wait_for_all_requests();

            //Drop on top of Kleavor (plenty of Scyther in the area as well)
            pbf_press_button(context, BUTTON_B, 50, 375);

            NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
            int ret2 = wait_until(
                console, context,
                std::chrono::seconds(45), //Enough time for Kleavor to notice and attack twice, sometimes it misses the first charge
                { battle_menu }
            );
            if (ret2 != 0) {
                console.log("Did not enter battle. Did Kleavor spawn?");
            }
        },
        {
            static_cast<VisualInferenceCallback&>(encounter_watcher),
            static_cast<AudioInferenceCallback&>(encounter_watcher),
        }
        );
    if (ret == 0) {
        console.log("Battle menu detected.");
    }

    encounter_watcher.throw_if_no_sound();

    bool is_shiny = (bool)encounter_watcher.shiny_screenshot();
    if (is_shiny) {
        console.log("Shiny detected!");
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
        throw ProgramFinishedException();
    } else {
        bool tera_self = true;
        wild_battle_tera(info, console, context, tera_self);
    }
    pbf_press_button(context, BUTTON_PLUS, 20, 105);
    return_to_plaza(info, console, context);

    //Day skip and attempt to respawn fixed encounters
    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    home_to_date_time(context, true, true);
    PokemonSwSh::roll_date_forward_1(context, true);
    resume_game_from_home(console, context);

    //Heal up and then reset position again.
    OverworldWatcher done_healing(COLOR_BLUE);
    pbf_move_left_joystick(context, 128, 0, 100, 20);

    pbf_mash_button(context, BUTTON_A, 300);
    context.wait_for_all_requests();

    int exit = run_until(
        console, context,
        [&](BotBaseContext& context){
            pbf_mash_button(context, BUTTON_B, 2000);
        },
        {{ done_healing }}
    );
    if (exit == 0){
        console.log("Overworld detected.");
    }
    open_map_from_overworld(info, console, context);
    pbf_press_button(context, BUTTON_ZL, 40, 100);
    fly_to_overworld_from_map(info, console, context);
}

void quest_sneak_up(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS) {
    EncounterWatcher encounter_watcher(console, COLOR_RED);
    console.log("Quest: Sneak up.");
    //Navigate to target and start battle
    int ret = run_until(
        console, context,
        [&](BotBaseContext& context) {
            //Savanna Plaza - Pride Rock
            open_map_from_overworld(info, console, context);
            pbf_move_left_joystick(context, 165, 255, 180, 20);
            pbf_press_button(context, BUTTON_ZL, 40, 100);
            fly_to_overworld_from_map(info, console, context);

            pbf_move_left_joystick(context, 220, 255, 10, 20);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            ssf_press_button(context, BUTTON_B, 0, 100);
            ssf_press_button(context, BUTTON_B, 0, 20, 10);
            ssf_press_button(context, BUTTON_B, 0, 20);
            pbf_wait(context, 100);
            context.wait_for_all_requests();
            pbf_press_button(context, BUTTON_LCLICK, 50, 0);

            if (BBQ_OPTIONS.INVERTED_FLIGHT) {
                pbf_move_left_joystick(context, 130, 255, 600, 250);
            }
            else {
                pbf_move_left_joystick(context, 130, 0, 600, 250);
            }

            pbf_wait(context, 400);
            context.wait_for_all_requests();
            pbf_press_button(context, BUTTON_B, 20, 50);
            pbf_wait(context, 400);
            context.wait_for_all_requests();

            pbf_press_button(context, BUTTON_PLUS, 20, 105);
            pbf_move_left_joystick(context, 255, 128, 20, 50);

            pbf_press_button(context, BUTTON_L, 20, 50);
            pbf_move_left_joystick(context, 128, 0, 100, 50);
            pbf_move_left_joystick(context, 0, 0, 20, 50);
            pbf_press_button(context, BUTTON_L, 20, 50);

            ssf_press_button(context, BUTTON_ZR, 0, 200);
            ssf_press_button(context, BUTTON_ZL, 100, 50);
            ssf_press_button(context, BUTTON_ZL, 150, 50);

            pbf_wait(context, 300);
            context.wait_for_all_requests();

            NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
            int ret2 = wait_until(
                console, context,
                std::chrono::seconds(45),
                { battle_menu }
            );
            if (ret2 != 0) {
                console.log("Did not enter battle. Did target spawn?");
            }
        },
        {
            static_cast<VisualInferenceCallback&>(encounter_watcher),
            static_cast<AudioInferenceCallback&>(encounter_watcher),
        }
        );
    if (ret == 0) {
        console.log("Battle menu detected.");
    }

    encounter_watcher.throw_if_no_sound();

    bool is_shiny = (bool)encounter_watcher.shiny_screenshot();
    if (is_shiny) {
        console.log("Shiny detected!");
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
        throw ProgramFinishedException();
    } else {
        OverworldWatcher overworld(COLOR_BLUE);

        int ret2 = run_until(
            console, context,
            [&](BotBaseContext& context) {
                while (true) {
                    //Flee immediately. Keep trying to flee.
                    //TODO: Add timer in case player gets stuck. Error out and check setup for smoke ball.
                    NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
                    int ret2 = wait_until(
                        console, context,
                        std::chrono::seconds(60),
                        { battle_menu }
                    );
                    switch (ret2) {
                    case 0:
                        battle_menu.move_to_slot(console, context, 3);
                        pbf_press_button(context, BUTTON_A, 10, 50);
                        break;
                    default:
                        console.log("Invalid state ret2 run_battle.");
                        throw OperationFailedException(
                            ErrorReport::SEND_ERROR_REPORT, console,
                            "Invalid state ret2 run_battle.",
                            true
                        );
                    }
                }
            },
            { overworld }
            );

        switch (ret2) {
        case 0:
            console.log("Overworld detected.");
            break;
        default:
            console.log("Invalid state in run_battle().");
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, console,
                "Invalid state in run_battle().",
                true
            );
        }
    }
    return_to_plaza(info, console, context);

    //Day skip and attempt to respawn fixed encounters - in case quest failed
    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    home_to_date_time(context, true, true);
    PokemonSwSh::roll_date_forward_1(context, true);
    resume_game_from_home(console, context);
    context.wait_for_all_requests();
}

void quest_wild_tera(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS) {
    EncounterWatcher encounter_watcher(console, COLOR_RED);
    console.log("Quest: Defeat a wild tera.");
    //Navigate to target and start battle
    int ret = run_until(
        console, context,
        [&](BotBaseContext& context) {
            //Canyon Rest Area
            open_map_from_overworld(info, console, context);
            pbf_move_left_joystick(context, 0, 140, 160, 20);
            pbf_press_button(context, BUTTON_ZL, 40, 100);
            fly_to_overworld_from_map(info, console, context);

            pbf_move_left_joystick(context, 255, 180, 20, 105);
            pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

            //Jump, glide, fly
            ssf_press_button(context, BUTTON_B, 0, 100);
            ssf_press_button(context, BUTTON_B, 0, 20, 10);
            ssf_press_button(context, BUTTON_B, 0, 20);
            pbf_wait(context, 100);
            context.wait_for_all_requests();
            pbf_press_button(context, BUTTON_LCLICK, 50, 0);

            if (BBQ_OPTIONS.INVERTED_FLIGHT) {
                pbf_move_left_joystick(context, 128, 255, 50, 250);
            }
            else {
                pbf_move_left_joystick(context, 128, 0, 500, 250);
            }

            pbf_wait(context, 1300);
            context.wait_for_all_requests();

            pbf_press_button(context, BUTTON_B, 50, 375);
            pbf_wait(context, 150);
            context.wait_for_all_requests();

            //Skarmory is likely to attack but sometimes there is a different pokemon
            pbf_press_button(context, BUTTON_PLUS, 20, 105);

            pbf_move_left_joystick(context, 50, 0, 20, 105);
            pbf_press_button(context, BUTTON_L, 20, 50);
            pbf_move_left_joystick(context, 128, 0, 100, 50);

            ssf_press_button(context, BUTTON_ZR, 0, 200);
            ssf_press_button(context, BUTTON_ZL, 100, 50);
            ssf_press_button(context, BUTTON_ZL, 150, 50);

            pbf_wait(context, 300);
            context.wait_for_all_requests();

            NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
            int ret2 = wait_until(
                console, context,
                std::chrono::seconds(45),
                { battle_menu }
            );
            if (ret2 != 0) {
                console.log("Did not enter battle.");
            }
        },
        {
            static_cast<VisualInferenceCallback&>(encounter_watcher),
            static_cast<AudioInferenceCallback&>(encounter_watcher),
        }
        );
    if (ret == 0) {
        console.log("Battle menu detected.");
    }

    encounter_watcher.throw_if_no_sound();

    bool is_shiny = (bool)encounter_watcher.shiny_screenshot();
    if (is_shiny) {
        console.log("Shiny detected!");
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
        throw ProgramFinishedException();
    } else {
        bool tera_self = false;
        wild_battle_tera(info, console, context, tera_self);
    }
    return_to_plaza(info, console, context);

    //Day skip and attempt to respawn fixed encounters
    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    home_to_date_time(context, true, true);
    PokemonSwSh::roll_date_forward_1(context, true);
    resume_game_from_home(console, context);

    //Heal up and then reset position again.
    OverworldWatcher done_healing(COLOR_BLUE);
    pbf_move_left_joystick(context, 128, 0, 100, 20);

    pbf_mash_button(context, BUTTON_A, 300);
    context.wait_for_all_requests();

    int exit = run_until(
        console, context,
        [&](BotBaseContext& context){
            pbf_mash_button(context, BUTTON_B, 2000);
        },
        {{ done_healing }}
    );
    if (exit == 0){
        console.log("Overworld detected.");
    }
    open_map_from_overworld(info, console, context);
    pbf_press_button(context, BUTTON_ZL, 40, 100);
    fly_to_overworld_from_map(info, console, context);
}

void quest_wash_pokemon(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context) {
    console.log("Quest: Give your pokemon a bath!");

    //Fly to savanna plaza
    open_map_from_overworld(info, console, context);
    pbf_move_left_joystick(context, 165, 255, 180, 20);
    pbf_press_button(context, BUTTON_ZL, 40, 100);
    fly_to_overworld_from_map(info, console, context);

    //Turn around, open picnic
    pbf_move_left_joystick(context, 128, 255, 20, 50);
    pbf_press_button(context, BUTTON_L, 50, 40);

    picnic_from_overworld(info, console, context);

    WallClock start = current_time();
    WhiteButtonWatcher wash_button(COLOR_BLUE, WhiteButton::ButtonX, {0.027, 0.548, 0.022, 0.032});
    WhiteButtonWatcher wash_exit_button(COLOR_BLUE, WhiteButton::ButtonB, {0.027, 0.923, 0.021, 0.033});
    WhiteButtonWatcher shower_switch(COLOR_BLUE, WhiteButton::ButtonY, {0.486, 0.870, 0.027, 0.045});
    bool rinsed_once = false;
    while (!rinsed_once) {
        context.wait_for_all_requests();
        if (current_time() - start > std::chrono::minutes(3)) {
            console.log("Failed to get to rinse after 3 minutes.", COLOR_RED);
            break;
        }
        int ret = wait_until(
            console, context, std::chrono::seconds(60),
            {
                wash_button,
                shower_switch,
                wash_exit_button,
            }
        );
        switch (ret) {
        case 0:
            console.log("Wash button found!");
            pbf_mash_button(context, BUTTON_X, 150);
            pbf_wait(context, 200);
            context.wait_for_all_requests();
            break;
        case 1:
            console.log("Rinse button found. Switching to rinse.");
            pbf_press_button(context, BUTTON_Y, 40, 50);
            rinsed_once = true;
            //Move slightly right, as the showerhead is at an angle
            pbf_move_left_joystick(context, 255, 0, 30, 30);
            context.wait_for_all_requests();
            break;
        case 2:
            console.log("In wash. Scrubbing.");

            ssf_press_button(context, BUTTON_A, 0, 200, 0);
            ssf_press_left_joystick(context, 0, 128, 0, 50);
            ssf_press_left_joystick(context, 255, 128, 50, 100);
            ssf_press_left_joystick(context, 0, 128, 150, 50);

            ssf_press_button(context, BUTTON_A, 0, 200, 0);
            ssf_press_left_joystick(context, 128, 0, 0, 50);
            ssf_press_left_joystick(context, 128, 255, 50, 100);
            ssf_press_left_joystick(context, 128, 0, 150, 50);
            pbf_wait(context, 400);
            context.wait_for_all_requests();
            break;
        }
    }

    WhiteButtonWatcher rinse_done(COLOR_BLUE, WhiteButton::ButtonY, {0.028, 0.923, 0.020, 0.034});
    WallClock start2 = current_time();
    int ret3 = run_until(
        console, context,
        [&](BotBaseContext& context) {
            while (true) {
                if (current_time() - start2 > std::chrono::minutes(1)) {
                    console.log("Failed to finish rinse after 1 minute.", COLOR_RED);
                    break;
                }
                ssf_press_button(context, BUTTON_A, 0, 200, 0);
                ssf_press_left_joystick(context, 0, 128, 0, 50);
                ssf_press_left_joystick(context, 255, 128, 50, 100);
                ssf_press_left_joystick(context, 0, 128, 150, 50);

                ssf_press_button(context, BUTTON_A, 0, 200, 0);
                ssf_press_left_joystick(context, 128, 0, 0, 50);
                ssf_press_left_joystick(context, 128, 255, 50, 100);
                ssf_press_left_joystick(context, 128, 0, 150, 50);
                pbf_wait(context, 400);
                context.wait_for_all_requests();
            }
        },
        { {rinse_done} }
    );
    if (ret3 == 0) {
        console.log("Shower completed successfully.");
    }
    else {
        console.log("Shower did not complete. Backing out.");
        pbf_press_button(context, BUTTON_B, 40, 50);
    }

    leave_picnic(info, console, context);

    return_to_plaza(info, console, context);
    context.wait_for_all_requests();
}

void quest_hatch_egg(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS) {
    console.log("Quest: Hatch an Egg");

    //Fly to Savanna Plaza and navigate to the battle court
    open_map_from_overworld(info, console, context);
    pbf_move_left_joystick(context, 165, 255, 180, 20);
    pbf_press_button(context, BUTTON_ZL, 40, 100);
    fly_to_overworld_from_map(info, console, context);

    pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);
    pbf_move_left_joystick(context, 128, 0, 500, 50);

    pbf_move_left_joystick(context, 0, 128, 20, 50);
    pbf_press_button(context, BUTTON_L, 20, 50);

    //Do this after navigating to prevent egg from hatchining enroute
    //Enter box system, navigate to left box, find egg, swap it with first pokemon in party
    enter_box_system_from_overworld(info, console, context);
    context.wait_for(std::chrono::milliseconds(400));
    
    //move_to_left_box(context);
    BoxCurrentEggDetector egg_detector;
    SomethingInBoxSlotDetector sth_in_box_detector(COLOR_RED);
    bool egg_found = false;
    uint8_t row = 0;
    uint8_t col = 0;
    for (row ; row < 5; row++){
        for (uint8_t j_col = 0; j_col < 6; j_col++){
            col = (row % 2 == 0 ? j_col : 5 - j_col);
            move_box_cursor(info, console, context, BoxCursorLocation::SLOTS, row, col);
            context.wait_for_all_requests();
            auto snapshot = console.video().snapshot();
            if (sth_in_box_detector.detect(snapshot) && egg_detector.detect(snapshot)){
                console.log("Found egg.");
                egg_found = true;
                break;
            }
        }
        if (egg_found) {
            break;
        }
    }

    swap_two_box_slots(info, console, context,
        BoxCursorLocation::SLOTS, row, col,
        BoxCursorLocation::PARTY, 0, 0);

    leave_box_system_to_overworld(info, console, context);

    hatch_eggs_anywhere(info, console, context, true, 1);

    enter_box_system_from_overworld(info, console, context);
    context.wait_for(std::chrono::milliseconds(400));

    swap_two_box_slots(info, console, context,
        BoxCursorLocation::PARTY, 0, 0,
        BoxCursorLocation::SLOTS, row, col);

    leave_box_system_to_overworld(info, console, context);

    pbf_press_button(context, BUTTON_PLUS, 20, 50);

    return_to_plaza(info, console, context);
    context.wait_for_all_requests();
}

void quest_sandwich(const ProgramInfo& info, AsyncDispatcher& dispatcher, ConsoleHandle& console, BotBaseContext& context, BBQOption& BBQ_OPTIONS, BBQuests& current_quest) {
    console.log("Quest: Make a singleplayer sandwich");

    bool flavored = true;
    //Polar Plaza - egg basket gets stuck under table in Savanna/Canyon Plaza
    open_map_from_overworld(info, console, context);
    pbf_move_left_joystick(context, 20, 25, 245, 20);
    pbf_press_button(context, BUTTON_ZL, 40, 100);
    fly_to_overworld_from_map(info, console, context);

    picnic_from_overworld(info, console, context);

    pbf_move_left_joystick(context, 128, 0, 30, 40);
    context.wait_for_all_requests();

    pbf_move_left_joystick(context, 128, 0, 70, 0);
    enter_sandwich_recipe_list(info, console, context);
    enter_custom_sandwich_mode(info, console, context);

    std::map<std::string, uint8_t> fillings;
    std::map<std::string, uint8_t> condiments;

    //Avoiding Encounter Power
    switch (current_quest) {
    case BBQuests::sour_sandwich:
        //Apple, Marmalade: Catch Flying 1, Item Drop Ice 1, Egg 1
        fillings = {{"apple", (uint8_t)1}};
        condiments = {{"marmalade", (uint8_t)1}};
        break;
    case BBQuests::sweet_sandwich:
        //Apple, Butter: Egg 1, Item Drop Ice 1, Raid Steel 1
        fillings = {{"apple", (uint8_t)1}};
        condiments = {{"butter", (uint8_t)1}};
        break;
    case BBQuests::spicy_sandwich:
        //Apple, Curry Powder: Raid Steel 1, Item Drop Ice 1, Teensy Flying 1
        fillings = {{"apple", (uint8_t)1}};
        condiments = {{"curry-powder", (uint8_t)1}};
        break;
    case BBQuests::salty_sandwich:
        //Cheese, Salt: Exp Point Electric 1, Raid Normal 1, Catching Psychic 1
        fillings = {{"cheese", (uint8_t)1}};
        condiments = {{"salt", (uint8_t)1}};
        break;
    case BBQuests::bitter_sandwich:
        //Watercress, Pepper: Item Drop Normal 1, Raid Power Flying 1, Exp Point Fighting 1
        fillings = {{"watercress", (uint8_t)1}};
        condiments = {{"pepper", (uint8_t)1}};
        break;
    case BBQuests::sandwich_three:
        //Fried Fillet, Noodles, Rice, Chili Sauce: Huge Water 1, Raid Flying 1, Catching Normal 1
        fillings = { {"fried-fillet", (uint8_t)1}, {"noodles", (uint8_t)1}, {"rice", (uint8_t)1} };
        condiments = {{"chili-sauce", (uint8_t)1}};
        flavored = false;
        break;
    }

    if (flavored) {
        make_bbq_flavored_sandwich(info, dispatcher, console, context, BBQ_OPTIONS.LANGUAGE, fillings, condiments);
    }
    else {
        make_bbq_three_sandwich(info, dispatcher, console, context, BBQ_OPTIONS.LANGUAGE, fillings, condiments);
    }

    leave_picnic(info, console, context);

    return_to_plaza(info, console, context);
    context.wait_for_all_requests();

}

}
}
}
