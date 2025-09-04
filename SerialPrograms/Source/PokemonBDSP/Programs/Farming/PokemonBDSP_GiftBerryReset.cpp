/*  Gift Berry Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <set>
#include "CommonFramework/Language.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP_GiftBerryReset.h"
#include "Pokemon/Inference/Pokemon_BerryNameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;

// The list of berries the Pastoria city berry npc offers.
const std::set<std::string> Pastoria_berry_list = {
    "occa-berry",
    "passho-berry",
    "wacan-berry",
    "rindo-berry",
    "yache-berry",
    "chople-berry",
    "kebia-berry",
    "shuca-berry",
    "coba-berry",
    "payapa-berry",
    "tanga-berry",
    "charti-berry",
    "kasib-berry",
    "haban-berry",
    "colbur-berry",
    "babiri-berry",
    "chilan-berry",
    "jaboca-berry",
    "rowap-berry",
    "roseli-berry",
};


GiftBerryReset_Descriptor::GiftBerryReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:GiftBerryReset",
        STRING_POKEMON + " BDSP", "Gift Berry Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/GiftBerryReset.md",
        "Reset the game in front of the NPC that gives rare berries in Pastoria City until a desired berry is received.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct GiftBerryReset_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_attempts(m_stats["Fetch Attempts"])
    {
        m_display_order.emplace_back("Fetch Attempts");
    }
    std::atomic<uint64_t>& m_attempts;
};
std::unique_ptr<StatsTracker> GiftBerryReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


GiftBerryReset::GiftBerryReset()
    : GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b><br>This is needed to read the berry name.",
        Pokemon::BerryNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , TARGET_BERRIES(
        "<b>Berries:</b><br>Multiple berries can be selected. The program will stop if one of the selected berries is received."
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(TARGET_BERRIES);
    PA_ADD_OPTION(NOTIFICATIONS);
}




void GiftBerryReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    GiftBerryReset_Descriptor::Stats& stats = env.current_stats<GiftBerryReset_Descriptor::Stats>();
    env.update_stats();

    //  Connect the controller.
    pbf_move_right_joystick(context, 0, 255, 10, 0);

    const auto selected_berries = TARGET_BERRIES.selected_berries();
    for (const auto& berry_slug: selected_berries){
        env.console.log("Target berry: " + berry_slug);
        if (Pastoria_berry_list.find(berry_slug) == Pastoria_berry_list.end()){
            throw UserSetupError(env.console, "The npc does not offer this berry: " + berry_slug);
        }
    }

    while (true){
        env.console.log("Talking to berry npc.");
        // Press ZL three times to advance dialog with npc
        for (int i = 0; i < 3; i++){
            pbf_mash_button(context, BUTTON_ZL, 30);
            pbf_wait(context, 150);
        }
        context.wait_for_all_requests();

        // Read dialog box to check which berry it is
        ShortDialogDetector dialog_detector;
        // VideoOverlaySet set(env.console);
        // dialog_detector.make_overlays(set);
        VideoSnapshot screen = env.console.video().snapshot();
        if (!dialog_detector.detect(screen)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "No npc dialog box found when reading berry name",
                env.console
            );
        }

        ImageFloatBox dialog_box(0.218, 0.835, 0.657, 0.12);
        ImageViewRGB32 dialog_image = extract_box_reference(screen, dialog_box);
        const auto result = Pokemon::BerryNameReader::instance().read_substring(
            env.console, LANGUAGE, dialog_image,
            OCR::BLACK_TEXT_FILTERS()
        );
        if (result.results.empty()){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "No berry name found in dialog box",
                env.console
            );
        }
        bool found_berry = false;
        for (const auto& r: result.results){
            env.console.log("Found potential berry name: " + r.second.token);

            if (TARGET_BERRIES.find_berry(r.second.token)){
                found_berry = true;
                break;
            }
        }

        stats.m_attempts++;
        env.update_stats();

        if (found_berry){
            env.console.log("Found one target berry. Stop program.");
            break;
        }

        // Reset game:
        pbf_press_button(context, BUTTON_HOME, 80ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
        if (!reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Error resetting game",
                env.console
            );
        }
    }

    env.update_stats();
    send_program_finished_notification(
        env, NOTIFICATION_PROGRAM_FINISH,
        "Match found!",
        env.console.video().snapshot()
    );
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}





}
}
}
