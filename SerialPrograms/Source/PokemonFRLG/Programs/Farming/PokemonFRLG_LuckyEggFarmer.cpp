/*  Lucky Egg Farmer
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
#include "PokemonFRLG/Inference/PokemonFRLG_BattleSelectionArrowDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_PokedexRegisteredDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_WildEncounterReader.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_LuckyEggFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

LuckyEggFarmer_Descriptor::LuckyEggFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:LuckyEggFarmer",
        Pokemon::STRING_POKEMON + " FRLG", "Lucky Egg Farmer",
        "Programs/PokemonFRLG/LuckyEggFarmer.html",
        "Farm the Lucky Egg from Chansey.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct LuckyEggFarmer_Descriptor::Stats : public StatsTracker {
    public:
    Stats()
        : chanseys(m_stats["Chanseys Caught"])
        , eggs(m_stats["Lucky Eggs Found"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Chanseys Caught");
        m_display_order.emplace_back("Lucky Eggs Found");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& chanseys;
    std::atomic<uint64_t>& eggs;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> LuckyEggFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

LuckyEggFarmer::LuckyEggFarmer()
    : LANGUAGE(
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
    , TAKE_VIDEO("<b>Take Video:</b><br>Record a video when the shiny is found.", LockMode::UNLOCK_WHILE_RUNNING, true)
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_SHINY(
        "Shiny found",
        true, true, ImageAttachmentMode::JPG,
        { "Notifs", "Showcase" }
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(STOP_AFTER_CURRENT);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(NOTIFICATIONS);
}

bool LuckyEggFarmer::navigate_to_chansey(ConsoleHandle& console, ProControllerContext& context){
    BlackScreenWatcher zone_exit(COLOR_RED);

    int ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
            ssf_press_button(context, BUTTON_B, 0ms, 4045ms);
            pbf_press_dpad(context, DPAD_UP, 100ms, 0ms);
            pbf_press_dpad(context, DPAD_RIGHT, 1966ms, 0ms);
            pbf_press_dpad(context, DPAD_UP, 1800ms, 0ms);
            pbf_press_dpad(context, DPAD_RIGHT, 450ms, 0ms);
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
        std::chrono::milliseconds(2000),
        { overworld_entered }
    );

    if (ret != 0){
        console.log("Failed to detect overworld after first zone transition.");
        return false;
    }

    console.log("Exiting first zone...");

    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();

    ret = run_until<ProControllerContext>(
        console, context,
        [](ProControllerContext& context) {
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
            pbf_press_dpad(context, DPAD_LEFT, 3000ms, 0ms);
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
        std::chrono::milliseconds(2000),
        { overworld_entered }
    );

    if (ret != 0){
        console.log("Failed to detect overworld after second zone transition.");
        return false;
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

void LuckyEggFarmer::swap_lead_pokemon(ConsoleHandle& console, ProControllerContext& context){
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

bool LuckyEggFarmer::find_encounter(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    ssf_press_button(context, BUTTON_B, 0ms, 400ms);
    pbf_press_dpad(context, DPAD_RIGHT, 400ms, 0ms);
    pbf_wait(context, 100ms);
    context.wait_for_all_requests();

    BlackScreenWatcher battle_entered(COLOR_RED);

    // This could be removed if spin in place stops drifting.
    AdvanceWhiteDialogWatcher out_of_steps(COLOR_RED);

    while (true) {
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context) {
                pbf_move_left_joystick(context, { +1, 0 }, 50ms, 150ms);
                context.wait_for_all_requests();
                pbf_move_left_joystick(context, { -1, 0 }, 33ms, 150ms);
                context.wait_for_all_requests();
            },
            { battle_entered, out_of_steps }
        );

        pbf_wait(context, 100ms);
        context.wait_for_all_requests();

        if (ret == 0) {
            env.log("Battle entered.");
            return true;
        }
        else if (ret == 1) {
            env.log("Out of steps dialog detected. Resetting...");
            return false;
        }
    }

    return false;
}

bool LuckyEggFarmer::is_chansey(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    std::set<std::string> subset = { "nidoran-f", "nidoran-m", "nidorino", "nidorina", "exeggcute", "rhyhorn", "venomoth", "chansey", "tauros" };

    WildEncounterReader reader(COLOR_RED);
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);

    env.log("Reading name...");
    VideoSnapshot screen = env.console.video().snapshot();
    PokemonFRLG_WildEncounter encounter = reader.read_encounter(env.logger(), LANGUAGE, screen, subset);
    env.log("Name: " + encounter.name);

    if (encounter.name != "chansey") {
        env.log("Not a Chansey. Fleeing...");
        flee_battle(env.console, context);
        context.wait_for_all_requests();
        return false;
    }
    else {
        env.log("Chansey found!");
        return true;
    }
}

bool LuckyEggFarmer::attempt_catch(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int& balls_left) {
    //TODO: Optimal bait/ball throwing

    while (true)
    {
        BattleSelectionArrowWatcher nickname_question_arrow(
            COLOR_RED,
            &env.console.overlay(),
            BattleConfirmationOption::YES
        );

        BattleSelectionArrowWatcher battle_arrow(
            COLOR_RED,
            &env.console.overlay(),
            SafariBattleMenuOption::BALL
        );

        BlackScreenWatcher battle_end(COLOR_RED);

        AdvanceBattleDialogWatcher advance_battle_dialog(COLOR_RED);

        PokedexRegisteredWatcher pokedex_registered(COLOR_RED, &env.console.overlay());

        WhiteDialogWatcher in_safari_zone_building(COLOR_RED);

        WallClock start = current_time();
        while (true)
        {
            if (current_time() - start > std::chrono::seconds(20)) {
                env.log("No battle activity detected for 20 seconds. Assuming battle ended and in the overworld.");

                //Check for safari zone building dialog?

                return false;
            }

            int ret = wait_until(
                env.console, context,
                std::chrono::milliseconds(2000),
                { nickname_question_arrow, battle_arrow, battle_end, advance_battle_dialog }
            );

            context.wait_for_all_requests();

            if (ret == 0 || ret == 3) {
                env.log("Caught a Chansey!");

                while (true)
                {
                    int ret2 = wait_until(
                        env.console, context,
                        std::chrono::milliseconds(2000),
                        { nickname_question_arrow, advance_battle_dialog, pokedex_registered, in_safari_zone_building }
                    );

                    if (ret2 == 0 || ret2 == 3) {
                        pbf_mash_button(context, BUTTON_B, 2000ms);
                        context.wait_for_all_requests();
                        break;
                    }
                    else if (ret2 == 1 || ret2 == 2) {
                        pbf_press_button(context, BUTTON_B, 200ms, 0ms);
                        context.wait_for_all_requests();
                    }
                }

                pbf_mash_button(context, BUTTON_B, 1500ms);
                context.wait_for_all_requests();
                return true;
            }
            else if (ret == 1) {
                balls_left--;
                env.log("Detected battle arrow. Balls left: " + std::to_string(balls_left));
                pbf_press_button(context, BUTTON_A, 200ms, 200ms);
                context.wait_for_all_requests();
                break;
            }
            else if (ret == 2) {
                env.log("Failed to catch Chansey.");
                pbf_wait(context, 1000ms);
                context.wait_for_all_requests();
                return false;
            }
        }
    }
}

bool LuckyEggFarmer::check_for_lucky_egg(ConsoleHandle& console, ProControllerContext& context, bool returned_to_building) {
    if (returned_to_building) {
        open_party_menu_from_overworld(console, context, StartMenuContext::STANDARD);
    } 
    else {
        open_party_menu_from_overworld(console, context, StartMenuContext::SAFARI_ZONE);
    }

    PartyHeldItemDetector held_item_detector(COLOR_RED, &console.overlay(), ImageFloatBox(0.432, 0.3, 0.030, 0.485));
    if (held_item_detector.detect(console.video().snapshot())) {
        return true;
    }

    return false;
}

void LuckyEggFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    home_black_border_check(env.console, context);

    LuckyEggFarmer_Descriptor::Stats& stats = env.current_stats<LuckyEggFarmer_Descriptor::Stats>();
    DeferredStopButtonOption::ResetOnExit reset_on_exit(STOP_AFTER_CURRENT);

    while (true){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        
        pbf_press_dpad(context, DPAD_UP, 200ms, 0ms);

        while (true)
        {
            BlackScreenWatcher safari_zone_exit(COLOR_RED);

            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context) {
                    pbf_press_button(context, BUTTON_A, 200ms, 200ms);
                },
                { safari_zone_exit }
            );

            if (ret == 0) {
                break;
            }
        }

        while (true) {
            BlackScreenOverWatcher overworld_entered(COLOR_RED);

            int ret = wait_until(
                env.console, context,
                std::chrono::milliseconds(2000),
                { overworld_entered }
            );

            if (ret == 0) {
                break;
            }
        }

        // There is a small delay from seeing the overworld to being able to actually move.
        pbf_wait(context, 2000ms);
        context.wait_for_all_requests();

        if (!navigate_to_chansey(env.console, context)){
            env.console.log("Navigation failed. Resetting...");
            soft_reset(env.console, context);
            continue;
        }

        swap_lead_pokemon(env.console, context);

        int chancy_count = 0;
        int balls_left = 30;

        while (chancy_count < 4) {
            if (!find_encounter(env, context)) {
                break;
            }

            bool encounter_shiny = handle_encounter(env.console, context, true);
            if (encounter_shiny) {
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
                if (TAKE_VIDEO) {
                    pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
                }
                return;
            }

            if (!is_chansey(env, context)) {
                continue;
            }

            bool caught = attempt_catch(env, context, balls_left);

            if (caught){
                stats.chanseys++;
                env.update_stats();
                chancy_count++;
            }

            pbf_wait(context, 500ms);
            context.wait_for_all_requests();

            WhiteDialogDetector dialog(COLOR_RED);
            bool in_safari_zone_building = dialog.detect(env.console.video().snapshot());
            
            if (balls_left <= 0) {
                in_safari_zone_building = true;
            }

            if (in_safari_zone_building && !caught) {
                break;
            }

            if (in_safari_zone_building) {
                pbf_mash_button(context, BUTTON_B, 500ms);
                context.wait_for_all_requests();
            }

            if (caught) {
                if (check_for_lucky_egg(env.console, context, in_safari_zone_building)) {
                    env.log("Lucky Egg found!");
                    stats.eggs++;
                    env.update_stats();
                    return;
                }
                env.log("Lucky Egg not found. Continuing to farm...");
                pbf_mash_button(context, BUTTON_B, 1500ms);
                context.wait_for_all_requests();
            }

            if (balls_left <= 0) {
                env.log("Out of Safari balls. Resetting...");
                break;
            }
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