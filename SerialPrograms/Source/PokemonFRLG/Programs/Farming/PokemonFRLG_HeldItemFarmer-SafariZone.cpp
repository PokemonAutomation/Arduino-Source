/*  Held Item Farmer - Safari Zone
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Options/ButtonOption.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_PartyHeldItemDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_BattleSelectionArrowDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_PokedexRegisteredDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_WildEncounterReader.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_HeldItemFarmer-SafariZone.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

HeldItemFarmerSafariZone_Descriptor::HeldItemFarmerSafariZone_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:HeldItemFarmerSafariZone",
        Pokemon::STRING_POKEMON + " FRLG", "Held Item Farmer - Safari Zone",
        "Programs/PokemonFRLG/HeldItemFarmer-SafariZone.html",
        "Farm held items from Chansey and Dragonair in the Safari Zone.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct HeldItemFarmerSafariZone_Descriptor::Stats : public StatsTracker {
    public:
    Stats()
        : encounters(m_stats["Encounters"])
        , target_pokemon_found(m_stats["Target Pokemon Found"])
        , target_pokemon_caught(m_stats["Target Pokemon Caught"])
        , items(m_stats["Items"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Encounters");
        m_display_order.emplace_back("Target Pokemon Found");
        m_display_order.emplace_back("Target Pokemon Caught");
        m_display_order.emplace_back("Items");
        m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_aliases["Lucky Eggs Found"] = "Items";
        m_aliases["Lucky Eggs"] = "Items";
        m_aliases["Chanseys Found"] = "Target Pokemon Found";
        m_aliases["Chanseys Caught"] = "Target Pokemon Caught";
    }

    std::atomic<uint64_t>& encounters;
    std::atomic<uint64_t>& target_pokemon_found;
    std::atomic<uint64_t>& target_pokemon_caught;
    std::atomic<uint64_t>& items;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> HeldItemFarmerSafariZone_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

HeldItemFarmerSafariZone::HeldItemFarmerSafariZone()
    : ITEM_TO_FARM(
        "<b>Item to Farm:</b>",
        {
            {ItemToFarm::LUCKY_EGG, "Lucky Egg", "Farm Chansey for Lucky Eggs."},
            {ItemToFarm::DRAGON_FANG, "Dragon Fang", "Farm Dragonair for Dragon Fangs."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        ItemToFarm::LUCKY_EGG
    )
    , LANGUAGE(
        "<b>Game Language:</b>",
        {
            Language::English,
            Language::Japanese,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
        },
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , STOP_AFTER_CURRENT("Reset")
    , TAKE_VIDEO(
        "<b>Take Video:</b><br>Record a video when a lucky egg or shiny is found.",
        LockMode::UNLOCK_WHILE_RUNNING, true
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_HELD_ITEM(
        "Held Item Found",
        true, true, ImageAttachmentMode::JPG,
        { "Notifs", "Showcase" }
    )
    , NOTIFICATION_SHINY(
        "Shiny found",
        true, true, ImageAttachmentMode::JPG,
        { "Notifs", "Showcase" }
    )
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_HELD_ITEM,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(ITEM_TO_FARM);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(STOP_AFTER_CURRENT);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(NOTIFICATIONS);
}

bool HeldItemFarmerSafariZone::navigate_to_chansey(ConsoleHandle& console, ProControllerContext& context){
    BlackScreenWatcher zone_exit(COLOR_RED);

    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 4045ms);
            pbf_press_dpad(context, DPAD_UP, 100ms, 0ms);
            pbf_press_dpad(context, DPAD_RIGHT, 1966ms, 0ms);
            pbf_press_dpad(context, DPAD_UP, 1800ms, 0ms);
            pbf_press_dpad(context, DPAD_RIGHT, 600ms, 0ms);
            pbf_wait(context, 1000ms);
        },
        { zone_exit }
    );

    context.wait_for_all_requests();

    if (ret != 0){
        console.log("Failed to detect zone transition after initial navigation.");
        return false;
    }

    BlackScreenOverWatcher overworld_entered(COLOR_RED);

    ret = wait_until(
        console, context,
        std::chrono::milliseconds(5000),
        { overworld_entered }
    );

    if (ret != 0){
        pbf_wait(context, 1000ms);
        context.wait_for_all_requests();

        console.log("Failed to detect overworld after first zone transition. Attempting to open Start menu as a secondary check.");
        StartMenuWatcher start_menu(COLOR_RED);

        context.wait_for_all_requests();
        ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_press_button(context, BUTTON_PLUS, 200ms, 1800ms);
            },
            { start_menu }
        );

        if (ret != 0){
            console.log("Failed to open Start menu.");
            context.wait_for_all_requests();
            pbf_mash_button(context, BUTTON_B, 1000ms);
            return false;
        }

        pbf_mash_button(context, BUTTON_B, 1000ms);
    }

    console.log("Exiting first zone...");

    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();

    ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 143005ms);
            pbf_press_dpad(context, DPAD_RIGHT, 1600ms, 0ms);
            pbf_press_dpad(context, DPAD_DOWN, 100ms, 0ms);
            pbf_press_dpad(context, DPAD_RIGHT, 2650ms, 0ms);
            pbf_press_dpad(context, DPAD_UP, 600ms, 0ms);
            pbf_press_dpad(context, DPAD_LEFT, 920ms, 0ms);
            pbf_press_dpad(context, DPAD_DOWN, 325ms, 0ms);
            pbf_press_dpad(context, DPAD_LEFT, 800ms, 0ms);
            pbf_press_dpad(context, DPAD_UP, 2000ms, 0ms);
            pbf_press_dpad(context, DPAD_RIGHT, 975ms, 0ms);
            pbf_press_dpad(context, DPAD_DOWN, 300ms, 0ms);
            pbf_press_dpad(context, DPAD_RIGHT, 550ms, 0ms);
            pbf_press_dpad(context, DPAD_UP, 675ms, 0ms);
            pbf_press_dpad(context, DPAD_LEFT, 2000ms, 0ms);
            pbf_press_dpad(context, DPAD_DOWN, 130ms, 0ms);
            pbf_press_dpad(context, DPAD_LEFT, 3300ms, 0ms);
        },
        { zone_exit }
    );

    if (ret != 0){
        console.log("Failed to detect zone transition while exiting second zone.");
        return false;
    }

    context.wait_for_all_requests();

    ret = wait_until(
        console, context,
        std::chrono::milliseconds(5000),
        { overworld_entered }
    );

    if (ret != 0){
        pbf_wait(context, 1000ms);
        context.wait_for_all_requests();

        console.log("Failed to detect overworld after second zone transition. Attempting to open Start menu as a secondary check.");
        StartMenuWatcher start_menu(COLOR_RED);

        context.wait_for_all_requests();
        ret = run_until<ProControllerContext>(
            console, context,
            [](ProControllerContext& context){
                pbf_press_button(context, BUTTON_PLUS, 200ms, 1800ms);
            },
            { start_menu }
        );

        if (ret != 0){
            console.log("Failed to open Start menu.");
            context.wait_for_all_requests();
            pbf_mash_button(context, BUTTON_B, 1000ms);
            return false;
        }

        pbf_mash_button(context, BUTTON_B, 1000ms);
    }

    console.log("Exiting second zone...");

    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, 2800ms);
    pbf_press_dpad(context, DPAD_LEFT, 2000ms, 0ms);
    pbf_press_dpad(context, DPAD_UP, 800ms, 0ms);

    context.wait_for_all_requests();

    return true;
}

void HeldItemFarmerSafariZone::navigate_to_dragonair(ConsoleHandle& console, ProControllerContext& context){
    ssf_press_button(context, BUTTON_B, 0ms, 2848ms);
    pbf_press_dpad(context, DPAD_UP, 1315ms, 0ms);
    pbf_press_dpad(context, DPAD_RIGHT, 755ms, 0ms);
    pbf_press_dpad(context, DPAD_UP, 580ms, 0ms);
}

void HeldItemFarmerSafariZone::swap_lead_pokemon(ConsoleHandle& console, ProControllerContext& context){
    open_party_menu_from_overworld(console, context, StartMenuContext::SAFARI_ZONE);
    pbf_press_button(context, BUTTON_A, 250ms, 100ms);
    pbf_press_dpad(context, DPAD_UP, 250ms, 100ms);
    pbf_press_dpad(context, DPAD_UP, 250ms, 100ms);
    pbf_press_dpad(context, DPAD_UP, 250ms, 100ms);
    pbf_press_button(context, BUTTON_A, 250ms, 100ms);
    pbf_press_dpad(context, DPAD_RIGHT, 250ms, 100ms);
    pbf_press_button(context, BUTTON_A, 250ms, 100ms);
    pbf_wait(context, 1000ms);
    pbf_mash_button(context, BUTTON_B, 1500ms);
    context.wait_for_all_requests();
}

bool HeldItemFarmerSafariZone::find_encounter_grass(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ssf_press_button(context, BUTTON_B, 0ms, 400ms);
    pbf_press_dpad(context, DPAD_RIGHT, 400ms, 0ms);
    pbf_wait(context, 100ms);
    context.wait_for_all_requests();

    BlackScreenWatcher battle_entered(COLOR_RED);
    AdvanceBattleDialogWatcher battle_dialog(COLOR_RED);

    // This could be removed if spin in place stops drifting.
    AdvanceWhiteDialogWatcher out_of_steps(COLOR_RED);

    while (true){
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, { +1, 0 }, 50ms, 150ms);
                context.wait_for_all_requests();
                pbf_move_left_joystick(context, { -1, 0 }, 33ms, 150ms);
                context.wait_for_all_requests();
            },
            { battle_entered, battle_dialog, out_of_steps }
        );

        pbf_wait(context, 100ms);
        context.wait_for_all_requests();

        switch (ret){
        case 0:
        case 1:
            env.log("Battle entered.");
            return true;
        case 2:
            env.log("Out of steps dialog detected. Resetting...");
            return false;
        }
    }
}

bool HeldItemFarmerSafariZone::find_encounter_fishing(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    WhiteDialogWatcher fishing_dialog(COLOR_RED);
    BlackScreenWatcher battle_entered(COLOR_RED);
    AdvanceBattleDialogWatcher battle_dialog(COLOR_RED);
    WallClock start = current_time();

    while (true){
        if (current_time() - start > std::chrono::seconds(300)){
            env.log("No pokemon hooked after 5 minutes. Resetting.");
            return false;
        }

        pbf_press_button(context, BUTTON_MINUS, 200ms, 200ms);
        context.wait_for_all_requests();

        int ret = wait_until(
            env.console, context,
            std::chrono::milliseconds(5000),
            { fishing_dialog, battle_entered, battle_dialog }
        );

        if (ret == 0){
            env.log("Fishing dialog detected.");
            pbf_press_button(context, BUTTON_B, 200ms, 200ms);
            context.wait_for_all_requests();
        } else if (ret == 1 || ret == 2){
            env.log("Battle entered.");
            return true;
        }
    }
}

bool HeldItemFarmerSafariZone::is_chansey(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    std::set<std::string> subset = {
        "nidoran-f",
        "nidoran-m",
        "nidorino",
        "nidorina",
        "exeggcute",
        "rhyhorn",
        "venomoth",
        "chansey",
        "tauros",
    };

    WildEncounterReader reader(COLOR_RED);
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);

    env.log("Reading name...");
    VideoSnapshot screen = env.console.video().snapshot();
    PokemonFRLG_WildEncounter encounter = reader.read_encounter(env.logger(), LANGUAGE, screen, subset);
    env.log("Name: " + encounter.name);

    return encounter.name == "chansey";
}

bool HeldItemFarmerSafariZone::is_dragonair(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    std::set<std::string> subset = {
        "goldeen",
        "seaking",
        "dratini",
        "psyduck",
        "slowpoke",
        "dragonair"
    };

    WildEncounterReader reader(COLOR_RED);
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);

    env.log("Reading name...");
    VideoSnapshot screen = env.console.video().snapshot();
    PokemonFRLG_WildEncounter encounter = reader.read_encounter(env.logger(), LANGUAGE, screen, subset);
    env.log("Name: " + encounter.name);

    return encounter.name == "dragonair";
}

bool HeldItemFarmerSafariZone::attempt_catch(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int& balls_left){
    //TODO: Optimal bait/ball throwing
    int distraction_thrown = 0;
    while (true){
        BattleSelectionArrowWatcher nickname_question_arrow(
            COLOR_RED,
            &env.console.overlay(),
            BattleConfirmationOption::YES
        );

        BattleSelectionArrowWatcher ball_arrow(
            COLOR_RED,
            &env.console.overlay(),
            SafariBattleMenuOption::BALL
        );

        BattleSelectionArrowWatcher bait_arrow(
            COLOR_RED,
            &env.console.overlay(),
            SafariBattleMenuOption::BAIT
        );

        BlackScreenWatcher battle_end(COLOR_RED);

        AdvanceBattleDialogWatcher advance_battle_dialog(COLOR_RED);

        PokedexRegisteredWatcher pokedex_registered(COLOR_RED, &env.console.overlay());

        WhiteDialogWatcher in_safari_zone_building(COLOR_RED);

        WallClock start = current_time();
        while (true){
            if (current_time() - start > std::chrono::seconds(20)){
                env.log("No battle activity detected for 20 seconds. Assuming battle ended and in the overworld.");

                //Check for safari zone building dialog?

                return false;
            }

            int ret = wait_until(
                env.console, context,
                std::chrono::milliseconds(2000),
                { nickname_question_arrow, ball_arrow, battle_end, advance_battle_dialog, bait_arrow }
            );

            context.wait_for_all_requests();

            if (ret == 0 || ret == 3){
                env.log("Pokemon Caught!");

                while (true){
                    int ret2 = wait_until(
                        env.console, context,
                        std::chrono::milliseconds(2000),
                        { nickname_question_arrow, advance_battle_dialog, pokedex_registered, in_safari_zone_building }
                    );

                    if (ret2 == 0 || ret2 == 3){
                        pbf_mash_button(context, BUTTON_B, 2000ms);
                        context.wait_for_all_requests();
                        break;
                    }
                    else if (ret2 == 1 || ret2 == 2){
                        pbf_press_button(context, BUTTON_B, 200ms, 0ms);
                        context.wait_for_all_requests();
                    }
                }

                pbf_mash_button(context, BUTTON_B, 1500ms);
                context.wait_for_all_requests();
                return true;
            }else if (ret == 1){
                if (distraction_thrown < 2){
                    env.log("Throwing distraction.");
                    pbf_press_dpad(context, DPAD_RIGHT, 200ms, 200ms);
                    pbf_press_button(context, BUTTON_A, 200ms, 200ms);
                    distraction_thrown++;
                    break;
                }

                balls_left--;
                env.log("Detected battle arrow. Balls left: " + std::to_string(balls_left));
                pbf_press_button(context, BUTTON_A, 200ms, 200ms);
                context.wait_for_all_requests();
                break;
            }else if (ret == 2){
                env.log("Failed to catch pokemon.");
                pbf_wait(context, 1000ms);
                context.wait_for_all_requests();
                return false;
            } else if (ret == 4){
                if (distraction_thrown < 2){
                    env.log("Throwing bait...");
                    pbf_press_button(context, BUTTON_A, 200ms, 200ms);
                    distraction_thrown++;
                    break;
                }
                env.log("Navigating to ball.");
                balls_left--;
                env.log("Balls left: " + std::to_string(balls_left));
                pbf_press_dpad(context, DPAD_LEFT, 200ms, 200ms);
                pbf_press_button(context, BUTTON_A, 200ms, 200ms);
                context.wait_for_all_requests();
                break;
            }
        }
    }
}

bool HeldItemFarmerSafariZone::check_for_held_item(ConsoleHandle& console, ProControllerContext& context, bool returned_to_building){
    if (returned_to_building){
        open_party_menu_from_overworld(console, context, StartMenuContext::STANDARD);
    } 
    else {
        open_party_menu_from_overworld(console, context, StartMenuContext::SAFARI_ZONE);
    }

    PartyHeldItemDetector held_item_detector(COLOR_RED, &console.overlay(), ImageFloatBox(0.432, 0.3, 0.030, 0.485));
    if (held_item_detector.detect(console.video().snapshot())){
        return true;
    }

    return false;
}

bool HeldItemFarmerSafariZone::run_safari_zone(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    HeldItemFarmerSafariZone_Descriptor::Stats& stats = env.current_stats<HeldItemFarmerSafariZone_Descriptor::Stats>();

    int chansey_count = 0;
    int balls_left = 30;

    while (chansey_count < 4){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        if (ITEM_TO_FARM == ItemToFarm::LUCKY_EGG){
            if (!find_encounter_grass(env, context)){
                return false;
            }
        }else{
            if (!find_encounter_fishing(env, context)){
                return false;
            }
        }
        

        bool encounter_shiny = handle_encounter(env.console, context, true);
        stats.encounters++;
        if (encounter_shiny){
            stats.shinies++;
            env.update_stats();
            send_program_notification(
                env,
                NOTIFICATION_SHINY,
                COLOR_YELLOW,
                "Shiny found!",
                {}, "",
                env.console.video().snapshot(),
                true
            );
            if (TAKE_VIDEO){
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
            }
            return true;
        }else{
            env.update_stats();
        }

        if (ITEM_TO_FARM == ItemToFarm::LUCKY_EGG){
            if (!is_chansey(env, context)){
                env.log("Not a Chansey. Fleeing...");
                flee_battle(env.console, context);
                context.wait_for_all_requests();
                continue;
            }
        }else{
            if (!is_dragonair(env, context)){
                env.log("Not a Dragonair. Fleeing...");
                flee_battle(env.console, context);
                context.wait_for_all_requests();
                continue;
            }
        }

        env.log("Target Pokemon found!");
        stats.target_pokemon_found++;
        env.update_stats();

        bool caught = attempt_catch(env, context, balls_left);

        if (caught){
            stats.target_pokemon_caught++;
            env.update_stats();
            chansey_count++;
        }

        pbf_wait(context, 500ms);
        context.wait_for_all_requests();

        WhiteDialogDetector dialog(COLOR_RED);
        bool in_safari_zone_building = dialog.detect(env.console.video().snapshot());

        if (balls_left <= 0){
            in_safari_zone_building = true;
        }

        if (in_safari_zone_building && !caught){
            return false;
        }

        if (in_safari_zone_building){
            pbf_mash_button(context, BUTTON_B, 500ms);
            context.wait_for_all_requests();
        }

        if (caught){
            if (check_for_held_item(env.console, context, in_safari_zone_building)){
                env.log("Held Item found!");
                stats.items++;
                env.update_stats();
                if (TAKE_VIDEO){
                    pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
                }
                send_program_notification(
                    env,
                    NOTIFICATION_HELD_ITEM,
                    Color(0xffffc0cb),
                    "Held Item found!",
                    {}, "",
                    env.console.video().snapshot(),
                    true
                );
                return true;
            }
            env.log("Held Item not found. Continuing to farm...");
            pbf_mash_button(context, BUTTON_B, 1500ms);
            context.wait_for_all_requests();
        }

        if (balls_left <= 0){
            env.log("Out of Safari balls. Resetting...");
            return false;
        }
    }

    return false;
}

void HeldItemFarmerSafariZone::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    home_black_border_check(env.console, context);

    DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_AFTER_CURRENT);

    while (true){
        pbf_press_dpad(context, DPAD_UP, 200ms, 0ms);

        while (true){
            BlackScreenWatcher safari_zone_exit(COLOR_RED);

            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_press_button(context, BUTTON_A, 200ms, 200ms);
                },
                { safari_zone_exit }
            );

            if (ret == 0){
                break;
            }
        }

        while (true){
            BlackScreenOverWatcher overworld_entered(COLOR_RED);

            int ret = wait_until(
                env.console, context,
                std::chrono::milliseconds(2000),
                { overworld_entered }
            );

            if (ret == 0){
                break;
            }
        }

        // There is a small delay from seeing the overworld to being able to actually move.
        pbf_wait(context, 2000ms);
        context.wait_for_all_requests();

        if (ITEM_TO_FARM == ItemToFarm::LUCKY_EGG){
            if (!navigate_to_chansey(env.console, context)){
                env.console.log("Navigation failed. Resetting...");
                soft_reset(env.console, context);
                continue;
            }

            swap_lead_pokemon(env.console, context);

        }else{ // Dragon Fang
            navigate_to_dragonair(env.console, context);
        }

        if (run_safari_zone(env, context)){
            GO_HOME_WHEN_DONE.run_end_of_program(context);
            return; // Already sent notification in run_safari_zone if shiny or lucky egg found.
        }

        soft_reset(env.console, context);

        if (STOP_AFTER_CURRENT.should_stop()){
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}


}
}
}
