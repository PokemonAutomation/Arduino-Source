/*  ESP Training
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/PokemonSV_ESPEmotionDetector.h"
#include "PokemonSV_ESPTraining.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;
 
ESPTraining_Descriptor::ESPTraining_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:ESPTraining",
        STRING_POKEMON + " SV", "ESP Training",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/ESPTraining.md",
        "Clear the ESP Training to farm EV berries.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
struct ESPTraining_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_emotions(m_stats["Emotions Shown"])
        , m_joy(m_stats["Joy"])
        , m_surprise(m_stats["Surprise"])
        , m_excitement(m_stats["Excitement"])
        , m_anger(m_stats["Anger"])
        , m_clears(m_stats["Times Cleared"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Emotions Shown");
        m_display_order.emplace_back("Joy");
        m_display_order.emplace_back("Surprise");
        m_display_order.emplace_back("Excitement");
        m_display_order.emplace_back("Anger");
        m_display_order.emplace_back("Times Cleared");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& m_emotions;
    std::atomic<uint64_t>& m_joy;
    std::atomic<uint64_t>& m_surprise;
    std::atomic<uint64_t>& m_excitement;
    std::atomic<uint64_t>& m_anger;
    std::atomic<uint64_t>& m_clears;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ESPTraining_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}
ESPTraining::ESPTraining()
    : ROUNDS(
        "<b>Number of times to run:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        10
    )
    , SAVE(
        "<b>Save game between rounds:</b><br>Save the game between ESP runs in case of crashes.",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        & NOTIFICATION_PROGRAM_FINISH,
        & NOTIFICATION_ERROR_FATAL,
        })
{
    PA_ADD_OPTION(ROUNDS);
    PA_ADD_OPTION(SAVE);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ESPTraining::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    ESPTraining_Descriptor::Stats& stats = env.current_stats<ESPTraining_Descriptor::Stats>();

    for (uint32_t c = 0; c < ROUNDS; c++){
        env.log("Round: " + tostr_u_commas(c));

        //Initiate dialog with Dendra
        //Dendra needs time to turn and face the player
        AdvanceDialogWatcher advance_detector(COLOR_YELLOW);
        pbf_press_button(context, BUTTON_A, 10, 50);
        int retD = wait_until(env.console, context, Milliseconds(4000), { advance_detector });
        if (retD < 0){
            env.log("Dialog detected.");
        }
        pbf_press_button(context, BUTTON_A, 10, 50);

        //Yes let's train
        pbf_press_button(context, BUTTON_A, 10, 50);
        pbf_wait(context, 100);
        context.wait_for_all_requests();

        //What mode? - Knockout
        pbf_press_button(context, BUTTON_A, 10, 50);
        pbf_press_dpad(context, DPAD_DOWN, 10, 50);
        pbf_press_button(context, BUTTON_A, 10, 50);
        context.wait_for_all_requests();

        //mash past other dialog
        pbf_mash_button(context, BUTTON_A, 360);
            
        //wait for start
        context.wait_for(std::chrono::milliseconds(13000));
        context.wait_for_all_requests();

        //Detect emotion and press the right button
        //151 emotions + pauses but the game is inconsistent and sometimes displays an emotion during the transitions
        //Note: can hit the wrong emotion and then the right one right after, as long as its before the timer
        bool endflag = true;
        while (endflag){
            ESPStartDetector ESPstart;
            ESPShowNewEmotionDetector ESPstop;
            ESPEmotionDetector detector;
            {
                //Countdown -> Dialog w/emotion
                int ret = wait_until(
                    env.console, context,
                    std::chrono::seconds(15),
                    { { ESPstart } }
                );
                if (ret < 0){
                    env.log("Timeout waiting for dialog.");
                    endflag = false;
                    break;
                }
                int ret2 = wait_until(
                    env.console, context,
                    std::chrono::seconds(5),
                    { { detector } }
                );
                if (ret2 < 0){
                    env.log("Timeout waiting for emotion.");
                }

                Button emotion_button = BUTTON_X;

                switch (detector.result()){
                case Detection::RED:
                    env.log("ESPEmotionDetector: Angry - Red - Press X", COLOR_BLACK);
                    emotion_button = BUTTON_X;
                    stats.m_emotions++;
                    stats.m_anger++;
                    break;
                case Detection::YELLOW:
                    env.log("ESPEmotionDetector: Joy - Yellow - Press A", COLOR_BLACK);
                    emotion_button = BUTTON_A;
                    stats.m_emotions++;
                    stats.m_joy++;
                    break;
                case Detection::BLUE:
                    env.log("ESPEmotionDetector: Surprised - Blue - Press B", COLOR_BLACK);
                    emotion_button = BUTTON_B;
                    stats.m_emotions++;
                    stats.m_surprise++;
                    break;
                case Detection::GREEN:
                    env.log("ESPEmotionDetector: Excited - Green - Press Y", COLOR_BLACK);
                    emotion_button = BUTTON_Y;
                    stats.m_emotions++;
                    stats.m_excitement++;
                    break;
                case Detection::GREY:
                    //Press any button to start next round
                    //Pressing A tends to make Dendra :D two extra times during the transition so press B instead
                    //Sometimes this is detected as blue, the B press there also works
                    env.log("ESPEmotionDetector: Grey - Mash though dialog", COLOR_BLACK);
                    emotion_button = BUTTON_B;
                    break;
                default:
                    endflag = false;
                    break;
                }

                // Press button and check it did not drop input. Press again if it did.
                // This will result in a duplicate press between phases, but the press will do nothing.
                pbf_press_button(context, emotion_button, 10, 50);
                env.update_stats();

                ESPPressedEmotionDetector emotion_press_detected;
                int check = wait_until(
                    env.console, context,
                    std::chrono::seconds(1),
                    { { emotion_press_detected } }
                );
                if (check < 0){
                    env.log("Emotion press not detected in bottom right. Pressing button again.");
                    pbf_press_button(context, emotion_button, 10, 50);
                }else{
                    env.log("Emotion press detected.");
                }

                //Look for the brief moment the dialog bubble vanishes
                ret = wait_until(
                    env.console, context,
                    std::chrono::seconds(15),
                    { { ESPstop } }
                );
                if (ret < 0){
                    env.log("Timeout waiting for dialog to vanish.");
                }
            }
            context.wait_for_all_requests();
        }

        //Program done, mash B until overworld detected
        OverworldWatcher overworld(env.console, COLOR_CYAN);
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 700);
            },
            {overworld}
        );
        if (ret != 0){
            env.console.log("Failed to detect overworld after ending.", COLOR_RED);
        }
        context.wait_for_all_requests();

        //Save the game if option checked, then loop again
        if(SAVE){
            save_game_from_overworld(env.program_info(), env.console, context);
        }

        stats.m_clears++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    }

    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}
    
}
}
}
