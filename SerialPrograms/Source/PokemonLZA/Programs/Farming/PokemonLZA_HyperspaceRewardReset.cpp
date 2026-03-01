/*  Hyperspace Reward Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_HyperspaceRewardNameReader.h"
#include "PokemonLZA/Resources/PokemonLZA_HyperspaceRewardNames.h"
#include "PokemonLZA_HyperspaceRewardReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;

HyperspaceRewardReset_Descriptor::HyperspaceRewardReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:HyperspaceRewardReset",
        STRING_POKEMON + " LZA", "Hyperspace Reward Reset",
        "Programs/PokemonLZA/HyperspaceRewardReset.html",
        "Reset in front of a Hyperspace Battle Zone trainer to receive a specific reward.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct HyperspaceRewardReset_Descriptor::Stats : public StatsTracker{
    Stats()
        : matches(m_stats["Items matched"])
        , resets(m_stats["Resets"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Items matched");
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& matches;
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> HyperspaceRewardReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

HyperspaceRewardReset::HyperspaceRewardReset()
    : LANGUAGE(
        "<b>Game Language:</b><br>The language is needed to read the rewards.",
        HyperspaceRewardNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , TARGET_ITEMS("<b>Items:</b>")
    , NOTIFICATION_REWARD_MATCH("Matching Reward", true, false, ImageAttachmentMode::JPG, { "Notifs" })
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_REWARD_MATCH,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(TARGET_ITEMS);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void HyperspaceRewardReset::talk_to_trainer(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    HyperspaceRewardReset_Descriptor::Stats& stats = env.current_stats<HyperspaceRewardReset_Descriptor::Stats>();

    bool exit = false;
    while (!exit){
        context.wait_for_all_requests();

        ButtonWatcher buttonA_watcher(
            COLOR_WHITE,
            ButtonType::ButtonA,
            {0.1, 0.1, 0.8, 0.8},
            &env.console.overlay()
        );
        //First dialog box varies by trainer (hologram, regular, etc.)
        BlueDialogWatcher blue_dialog_watcher(COLOR_BLUE, &env.console.overlay()); //Item received (not the big box)
        FlatWhiteDialogWatcher white_dialog_watcher(COLOR_WHITE, &env.console.overlay()); //Non-hologram
        LightBlueDialogWatcher light_blue_dialog_watcher(COLOR_WHITE, &env.console.overlay()); //hologram
        
        int ret = wait_until(
            env.console, context,
            10s,
            {
                buttonA_watcher,
                blue_dialog_watcher,
                white_dialog_watcher,
                light_blue_dialog_watcher,
            }
        );
        context.wait_for(100ms);

        switch (ret){
        case 0:
            env.log("Detected A button.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 1:
            env.log("Detected item received dialog.");
            exit = true;
            break;
        case 2:
            env.log("Detected white dialog.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        case 3:
            env.log("Detected light blue dialog.");
            pbf_press_button(context, BUTTON_A, 80ms, 40ms);
            continue;
        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "talk_to_trainer(): Failed to detect blue item received dialog.",
                env.console
            );
        }
    }
}

//Check reward and notify if it matches filters
bool HyperspaceRewardReset::check_reward(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    HyperspaceRewardReset_Descriptor::Stats& stats = env.current_stats<HyperspaceRewardReset_Descriptor::Stats>();

    VideoSnapshot screen = env.console.video().snapshot();
    ImageFloatBox dialog_box(0.248, 0.792, 0.483, 0.147);
    ImageViewRGB32 dialog_image = extract_box_reference(screen, dialog_box);

    /*
    bool replace_color_within_range = false;
    ImageRGB32 dialog_filtered = filter_rgb32_range(
        extract_box_reference(screen, dialog_box),
        combine_rgb(50, 135, 162), combine_rgb(167, 244, 255), Color(0), replace_color_within_range
    );
    dialog_filtered.save("./dialog_image.png");
    */

    const double LOG10P_THRESHOLD = -1.5;
    OCR::StringMatchResult result = PokemonLZA::HyperspaceRewardNameReader::instance().read_substring(
        env.console, LANGUAGE, dialog_image,
        OCR::BLUE_TEXT_FILTERS()
    );
    result.clear_beyond_log10p(LOG10P_THRESHOLD);
    if (result.results.empty()){
        env.log("No matching reward name found in dialog box.");
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    }
    for (const auto& r : result.results){
        env.console.log("Found reward: " + r.second.token);
        if (TARGET_ITEMS.find_item(r.second.token)){
            env.log("Reward Matched");

            stats.matches++;
            env.update_stats();

            send_program_notification(
                env, NOTIFICATION_REWARD_MATCH,
                COLOR_GREEN, "Reward Matched",
                {
                    { "Item:", get_hyperspace_reward_name(r.second.token).display_name() },
                }
            , "", screen);
            return true;
            break;
        }
    }
    return false;
}

void HyperspaceRewardReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    HyperspaceRewardReset_Descriptor::Stats& stats = env.current_stats<HyperspaceRewardReset_Descriptor::Stats>();

    /*
    * In a Hyperspace Battle Zone:
    * Win a battle with a trainer that gives a prize.
    * Save the game and reset. Do not talk to the trainer.
    * (Reset is required because trainer position will move.)
    * Re-enter the game and then save again, this time in front of the trainer.
    * Start the program.
    * 
    * Make sure you won't be spotted by another trainer.
    */

    bool item_found = false;
    while (!item_found){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        talk_to_trainer(env, context);
        item_found = check_reward(env, context);

        if (item_found){
            break;
        }

        go_home(env.console, context);
        reset_game_from_home(env, env.console, context);
        stats.resets++;
        env.update_stats();
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    go_home(env.console, context);
}

}
}
}

