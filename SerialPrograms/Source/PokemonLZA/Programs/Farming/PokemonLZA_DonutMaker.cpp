/*  Donut Maker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
#include "PokemonLZA/Resources/PokemonLZA_DonutBerries.h"
#include "PokemonLZA/Inference/Donuts/PokemonLZA_DonutBerriesDetector.h"
#include "PokemonLZA/Inference/Donuts/PokemonLZA_DonutPowerDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_DonutBerrySession.h"
#include "PokemonLZA_DonutMaker.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {

using namespace Pokemon;


DonutMaker_Descriptor::DonutMaker_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:DonutMaker", STRING_POKEMON + " LZA",
        "Donut Maker",
        "Programs/PokemonLZA/DonutMaker.html",
        "TODO: Add description",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

class DonutMaker_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Resets"])
        , errors(m_stats["Errors"])
    {
        // TODO: Add more stats here
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> DonutMaker_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


DonutMaker::DonutMaker()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , BERRIES("<b>Berries:</b><br>The berries used to make the donut. Minimum 3 berries, maximum 8 berries.")
    , NUM_POWER_REQUIRED(
        "<b>Number of Powers to Match:</b><br>How many of a donut's powers must be in the the table below. Minimum 1, maximum 3. "
        "<br>Ex. For a target donut of Big Haul Lv.3, Berry Lv.3, and any or none for the 3rd power, set the number as 2."
        "<br>Then, in the flavor powers table, make sure to add Big Haul Lv.3 and Berry Lv. 3.",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 3
        )
    //, NUM_DONUTS(
    //    "<b>Number of Donuts:</b><br>The number of donuts to make.",
    //    LockMode::LOCK_WHILE_RUNNING,
    //    1, 1
    //)
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(BERRIES);
    PA_ADD_OPTION(NUM_POWER_REQUIRED);
    PA_ADD_OPTION(FLAVOR_POWERS);
    //PA_ADD_OPTION(NUM_DONUTS); //TODO: Add looping. Navigate back to PC and heal to make backup save.
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

bool DonutMaker::match_powers(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    env.log("Reading in table of desired powers.");
    std::vector<std::string> power_table;
    std::vector<std::unique_ptr<FlavorPowerTableRow>> wanted_powers_table = FLAVOR_POWERS.copy_snapshot();
    for (const std::unique_ptr<FlavorPowerTableRow>& row : wanted_powers_table){
        FlavorPowerTableEntry table_line = row->snapshot();
        power_table.push_back(table_line.to_str());
        env.log(table_line.to_str());
    }

    env.log("Reading powers and counting up hits.");
    uint8_t num_hits = 0;
    VideoSnapshot screen = env.console.video().snapshot();
    for (int i = 0; i < 3; i++) {
        DonutPowerDetector read_power(env.logger(), COLOR_GREEN, LANGUAGE, i);
        if ((std::find(power_table.begin(), power_table.end(), read_power.detect_quest(screen)) != power_table.end())) {
            num_hits++;
        }
    }

    if (num_hits >= NUM_POWER_REQUIRED) {
        return true;
    }

    return false;
}

void DonutMaker::animation_to_donut(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    WhiteScreenOverWatcher animation_skip(COLOR_BLUE);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, Seconds(3));
            pbf_wait(context, Seconds(30));
        },
        {animation_skip}
    );
    context.wait_for_all_requests();
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
           ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Unable to skip donut making animation.",
            env.console
        );
    }

    //Press A and wait a bit for powers to appear
    pbf_wait(context, Seconds(3));
    pbf_press_button(context, BUTTON_A, 100ms, 200ms);
    pbf_wait(context, Seconds(3));
    context.wait_for_all_requests();
}

void DonutMaker::add_berries_in_menu_and_start(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    //DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    env.log("Checking berry count.");
    std::vector<std::unique_ptr<DonutBerriesTableRow>> berries_table = BERRIES.copy_snapshot();
    env.log("Number of berries validated.", COLOR_BLACK);

    //Berries map for selection
    std::map<std::string, uint8_t> processed_berries;
    for (const std::unique_ptr<DonutBerriesTableRow>& row : berries_table){
        const std::string& table_item = row->berry.slug();
        env.log("Adding: " + table_item);
        processed_berries[table_item]++;
    }
    //cout << "Processed Berries:" << endl;
    //for (const auto& [key, value] : processed_berries){
    //    std::cout << key << ": " << (int)value << endl;
    //}

    add_donut_ingredients(env.console, context, LANGUAGE, std::move(processed_berries));
    animation_to_donut(env, context);
}

// Press A to talk to Ansha and keep pressing A until reach the berry selection menu
void DonutMaker::open_berry_menu_from_ansha(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    // press button A to start talking to Ansha
    pbf_press_button(context, BUTTON_A, 100ms, 200ms);
    context.wait_for_all_requests();
    
    WallClock start = current_time();
    while(current_time() - start <= Seconds(120)){
        FlatWhiteDialogWatcher white_dialog(COLOR_WHITE, &env.console.overlay());
        SelectionArrowWatcher arrow(
            COLOR_GREEN, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.591, 0.579, 0.231, 0.105}
        );
        DonutBerriesSelectionWatcher berry_selection(0);

        int ret = wait_until(env.console, context, std::chrono::seconds(3),
            {white_dialog, arrow, berry_selection});
        switch (ret){
        case 0:
            env.log("Detected white dialog. Go to next dialog");
            pbf_press_button(context, BUTTON_A, 100ms, 200ms);
            break;
        case 1:
            env.log("Detected selection arrow. Go to next dialog");
            pbf_press_button(context, BUTTON_A, 100ms, 200ms);
            break;
        case 2:
            env.log("Berry selection menu shown.");
            env.add_overlay_log("Found Berry Selection Menu");
            return;
        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "donut_maker(): Unable to detect white dialog, selection arrow or berry menu after talking to Ansha.",
                env.console
            );
        } // end switch(ret)
        context.wait_for_all_requests();
    } // end while(true)

    stats.errors++;
    env.update_stats();
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "donut_maker(): 2 minutes passed yet unable to reach berry menu after taking to Ansha.",
        env.console
    );
}


// Move from pokecenter to in front of Ansha with button A shown
void move_from_pokecenter_to_ansha(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    bool zoom_to_max = false;
    open_map(env.console, context, zoom_to_max);
    // Move map cursor upwards a little bit so that the cursor locks onto the pokecenter.
    // This is needed so that in the fast travel location menu the Hotel Z is one row near
    // the default position on the menu.
    pbf_move_left_joystick_old(context, 128, 64, 100ms, 200ms);
    // Press Y to load fast travel locaiton menu. The cursor should now points to Vert Pokemon Center
    pbf_press_button(context, BUTTON_Y, 125ms, 300ms);
    // Move one menu item up to select Hotel Z
    pbf_press_dpad(context, DPAD_UP, 50ms, 200ms);
    context.wait_for_all_requests();

    OverworldPartySelectionWatcher overworld(COLOR_WHITE, &env.console.overlay());
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, Seconds(10));
            pbf_wait(context, Seconds(30)); // 30 sec to wait out potential day night change
        },
        {overworld}
    );
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
           ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Unable to find overworld after fast traveling from Vert Pokemon Center after 30 sec.",
            env.console
        );
    }

    ret = run_towards_gate_with_A_button(env.console, context, 128, 0, Seconds(5));
    if (ret == 1){  // day night change happens during running
        // As day night change has ended, try running towards door again
        if (run_towards_gate_with_A_button(env.console, context, 128, 0, Seconds(5)) != 0){
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "donut_maker(): Cannot reach Hotel Z gate after day/night change.",
                env.console
            );
        }
    } else if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Cannot reach Hotel Z gate after fast travel.",
            env.console
        );
    }

    // Mash button A to enter the hotel.
    pbf_mash_button(context, BUTTON_A, Seconds(2));
    context.wait_for_all_requests();
    WallClock start_time = current_time();
    // We use 50s here to account for day night change
    wait_until_overworld(env.console, context, 50s);
    env.console.log("Detected overworld after entering zone.");
    WallClock end_time = current_time();
    const auto duration = end_time - start_time;
    // Due to day/night change may eating the mashing button A sequence, we may still be outside the hotel!
    if (duration >= 16s){
        // mash A again to make sure we are inside the hotel
        pbf_mash_button(context, BUTTON_A, Seconds(2));
        context.wait_for_all_requests();
        wait_until_overworld(env.console, context, 50s);
    }

    
    // we are now inside the hotel

    // Roll forward twice
    pbf_press_button(context, BUTTON_Y, 100ms, 1s);
    pbf_press_button(context, BUTTON_Y, 100ms, 1s);
    pbf_move_left_joystick(context, {-1, 0}, 500ms, 100ms);
    context.wait_for_all_requests();
}


// Return true if it should stop
// Start the iteration at closest pokemon center
bool DonutMaker::donut_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    move_from_pokecenter_to_ansha(env, context);

    const ImageFloatBox button_A_box{0.3, 0.2, 0.4, 0.7};
    ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, button_A_box, &env.console.overlay());
    int ret = wait_until(env.console, context, std::chrono::seconds(3), {buttonA});
    if (ret != 0){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Unable to find button A facing Ansha.",
            env.console
        );
    }

    open_berry_menu_from_ansha(env, context);
    add_berries_in_menu_and_start(env, context);

    if (match_powers(env, context)){
        return true;
    }

    go_home(env.console, context);
    const bool backup_save = true;
    if (!reset_game_from_home(env, env.console, context, backup_save)){
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "donut_maker(): Cannot reset game from Switch Home screen.",
            env.console
        );
    }
    return false;
}


void DonutMaker::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DonutMaker_Descriptor::Stats& stats = env.current_stats<DonutMaker_Descriptor::Stats>();

    assert_16_9_1080p_min(env.logger(), env.console);

    if (LANGUAGE == Language::None){
        throw UserSetupError(env.console, "Must set game language option to read berries and flavor powers.");
    }

    {
        const std::vector<std::unique_ptr<DonutBerriesTableRow>> berries_table = BERRIES.copy_snapshot();
        const size_t num_berries = berries_table.size();
        if (num_berries < 3 || num_berries > 8) {
            throw UserSetupError(env.console, "Must have at least 3 berries and no more than 8 berries.");
        }
    }

    //Print table to log to check
    //std::vector<std::unique_ptr<FlavorPowerTableRow>> wanted_powers_table = FLAVOR_POWERS.copy_snapshot();
    //for (const std::unique_ptr<FlavorPowerTableRow>& row : wanted_powers_table){
    //    FlavorPowerTableEntry table_line = row->snapshot();
    //    env.log(table_line.to_str());
    //}
    //TODO: Validate powers. The "All Types" type only applies to catching and sparkling powers. Move and resist do not have "All Types"
    //Are people even going to target move and resist power? Big Haul/Item Berry/Alpha/Sparkling seems more likely.
    //also TODO: powers table to however we're going to check it once that is written

    //  Mash button B to let Switch register the controller
    pbf_mash_button(context, BUTTON_B, 200ms);

    while(true){
        const bool should_stop = donut_iteration(env, context);
        stats.resets++;
        env.update_stats();

        if (should_stop){
            break;
        }
    }


    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}  // namespace PokemonLZA
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
