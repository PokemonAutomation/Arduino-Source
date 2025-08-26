/*  Egg Autonomous State
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/FrozenImageDetector.h"
#include "CommonTools/VisualDetectors/ImageMatchDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxGenderDetector.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxNatureDetector.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxShinyDetector.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_IvJudgeReader.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameNavigation.h"
#include "PokemonBDSP_EggRoutines.h"
#include "PokemonBDSP_EggFeedback.h"
#include "PokemonBDSP_EggAutonomousState.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


EggAutonomousStats::EggAutonomousStats()
    : m_hatched(m_stats["Eggs Hatched"])
    , m_errors(m_stats["Errors"])
    , m_fetch_attempts(m_stats["Fetch Attempts"])
    , m_fetch_success(m_stats["Fetch Success"])
    , m_shinies(m_stats["Shinies"])
{
    m_display_order.emplace_back("Eggs Hatched");
    m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    m_display_order.emplace_back("Fetch Attempts");
    m_display_order.emplace_back("Fetch Success");
    m_display_order.emplace_back("Shinies");
}




class EggReceivedDetector : public VisualInferenceCallback{
public:
    EggReceivedDetector(Color color = COLOR_RED)
        : VisualInferenceCallback("EggReceivedDetector")
        , m_fetched(false)
        , m_color(color)
        , m_box0(0.05, 0.10, 0.10, 0.80)
        , m_box1(0.87, 0.10, 0.10, 0.80)
    {}

    bool fetched() const{
        return m_fetched;
    }

    virtual void make_overlays(VideoOverlaySet& items) const override{
        items.add(m_color, m_box0);
        items.add(m_color, m_box1);
    }
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override{
        ImageStats stats0 = image_stats(extract_box_reference(frame, m_box0));
        ImageStats stats1 = image_stats(extract_box_reference(frame, m_box1));
        if (!is_solid(stats0, {0.22951, 0.340853, 0.429638}, 0.15, 20)){
            return false;
        }
        if (!is_solid(stats1, {0.22951, 0.340853, 0.429638}, 0.15, 20)){
            return false;
        }
        m_fetched = true;
        return false;
    }

private:
    bool m_fetched;
    Color m_color;
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
};



EventNotificationOption EggAutonomousState::m_notification_noop("", false, false);


EggAutonomousState::EggAutonomousState(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    EggAutonomousStats& stats,
    EventNotificationOption& notification_nonshiny_keep,
    EventNotificationOption& notification_shiny,
    Milliseconds scroll_to_read_delay,
    Language language,
    ShortcutDirectionOption& shortcut,
    Milliseconds travel_time_per_fetch,
    const StatsHuntIvJudgeFilterTable& filters,
    uint8_t max_keepers,
    uint8_t existing_eggs_in_columns
)
    : m_env(env), m_stream(stream), m_context(context)
    , m_stats(stats)
    , m_notification_nonshiny_keep(notification_nonshiny_keep)
    , m_notification_shiny(notification_shiny)
    , m_scroll_to_read_delay(scroll_to_read_delay)
    , m_language(language)
    , m_shortcut(shortcut)
    , m_travel_time_per_fetch(travel_time_per_fetch)
    , m_filters(filters)
    , m_max_keepers(max_keepers)
    , m_eggs_in_column(existing_eggs_in_columns)
{}

void EggAutonomousState::dump() const{
    std::string str = "Current State:\n";
    str += "    On Bike: ";
    str += (m_on_bike ? "Yes" : "No");
    str += "\n    Box Column: " + std::to_string(m_eggs_in_column);
    str += "\n    Party Eggs: " + std::to_string(m_eggs_in_party);
    str += "\n    Babies Saved: " + std::to_string(m_babies_saved);
    m_stream.log(str);
}
void EggAutonomousState::set(const EggAutonomousState& state){
    m_on_bike = state.m_on_bike;
    m_eggs_in_column = state.m_eggs_in_column;
    m_eggs_in_party = state.m_eggs_in_party;
    m_babies_saved = state.m_babies_saved;
}

void EggAutonomousState::process_error(const std::string& name, const char* message){
    m_stats.m_errors++;
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        message,
        m_stream
    );
}

void EggAutonomousState::process_shiny(const ImageViewRGB32& screen){
//    take_video(m_console);
    m_stats.m_shinies++;
    m_env.update_stats();
    send_encounter_notification(
        m_env,
        m_notification_noop,
        m_notification_shiny,
        false, true, {{{}, ShinyType::UNKNOWN_SHINY}}, std::nan(""),
        screen
    );
}


void EggAutonomousState::withdraw_egg_column(){
    m_stream.log("Withdrawing column from box to your party...");

    const Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;
    const Milliseconds BOX_PICKUP_DROP_DELAY = GameSettings::instance().BOX_PICKUP_DROP_DELAY0;

    pbf_press_button(m_context, BUTTON_Y, 20, 50);
    pbf_press_button(m_context, BUTTON_Y, 20, 50);
    pickup_column(m_context);
    pbf_move_right_joystick(m_context, 0, 128, 160ms, BOX_SCROLL_DELAY);
    pbf_move_right_joystick(m_context, 128, 255, 160ms, BOX_SCROLL_DELAY);
    pbf_press_button(m_context, BUTTON_ZL, 160ms, BOX_PICKUP_DROP_DELAY);

    m_eggs_in_column = 0;
    m_eggs_in_party = 5;
}
bool EggAutonomousState::process_party(){
    m_stream.log("Processing party...");

    const Milliseconds BOX_SCROLL_DELAY = GameSettings::instance().BOX_SCROLL_DELAY0;

    pbf_move_right_joystick(m_context, 0, 128, 160ms, BOX_SCROLL_DELAY);
    pbf_move_right_joystick(m_context, 128, 255, 160ms, BOX_SCROLL_DELAY);
    pbf_wait(m_context, m_scroll_to_read_delay);
    m_context.wait_for_all_requests();
//    m_env.wait_for(SCROLL_TO_READ_DELAY);

    BoxShinyDetector shiny_reader;
    IvJudgeReaderScope iv_reader(m_stream.overlay(), m_language);
    BoxNatureDetector nature_detector(m_stream.overlay(), m_language);

    VideoOverlaySet set(m_stream.overlay());
    shiny_reader.make_overlays(set);

    //  Make sure the stats menu is up.
    VideoSnapshot screen = m_stream.video().snapshot();
    if (!shiny_reader.is_panel(screen)){
        process_error("StatsPanel", "Stats panel not detected.");
    }

    //  Run through the 5 hatchlings and release all the non-shinies.
    for (size_t c = 0; c < 5; c++){
        if (c != 0){
            pbf_move_right_joystick(m_context, 128, 0, 160ms, BOX_SCROLL_DELAY);
            pbf_move_right_joystick(m_context, 128, 255, 160ms, BOX_SCROLL_DELAY);
            pbf_wait(m_context, m_scroll_to_read_delay);
            m_context.wait_for_all_requests();
//            m_env.wait_for(SCROLL_TO_READ_DELAY);

            screen = m_stream.video().snapshot();
            if (!shiny_reader.is_panel(screen)){
                process_error("StatsPanel", "Stats panel not detected.");
            }
        }
//        m_context.wait_for_all_requests();

        bool shiny = shiny_reader.detect(screen);
        if (shiny){
            m_stream.log("Pokemon " + std::to_string(c) + " is shiny!", COLOR_BLUE);
            process_shiny(screen);
        }else{
            m_stream.log("Pokemon " + std::to_string(c) + " is not shiny.", COLOR_PURPLE);
        }
        IvJudgeReader::Results IVs = iv_reader.read(m_stream.logger(), screen);
        StatsHuntGenderFilter gender = read_gender_from_box(m_stream.logger(), m_stream.overlay(), screen);
        NatureReader::Results nature = nature_detector.read(m_stream.logger(), screen);

        StatsHuntAction action = m_filters.get_action(shiny, gender, nature.nature, IVs);

        switch (action){
        case StatsHuntAction::StopProgram:
            m_stream.log("Program stop requested...");
            if (!shiny){
                send_encounter_notification(
                    m_env,
                    m_notification_nonshiny_keep,
                    m_notification_shiny,
                    false, false, {}, std::nan(""),
                    screen
                );
            }
            return true;
        case StatsHuntAction::Keep:
            m_stream.log("Moving Pokemon to keep box...", COLOR_BLUE);
            if (!shiny){
                send_encounter_notification(
                    m_env,
                    m_notification_nonshiny_keep,
                    m_notification_shiny,
                    false, false, {}, std::nan(""),
                    screen
                );
            }
            pbf_press_button(m_context, BUTTON_ZL, 20, 105);
            pbf_press_button(m_context, BUTTON_ZL, 20, 105);
            pbf_move_right_joystick(m_context, 128, 0, 20, 105);
            pbf_move_right_joystick(m_context, 128, 0, 20, 105);
            pbf_move_right_joystick(m_context, 128, 0, 20, 105);
            pbf_move_right_joystick(m_context, 255, 128, 20, 105);
            pbf_press_button(m_context, BUTTON_ZL, 20, 230);
            pbf_move_right_joystick(m_context, 0, 128, 20, 105);
            pbf_move_right_joystick(m_context, 128, 0, 20, 105);
            pbf_move_right_joystick(m_context, 128, 0, 20, 105);
            pbf_press_button(m_context, BUTTON_ZL, 20, 105);
            pbf_press_button(m_context, BUTTON_B, 20, 230);
            pbf_press_button(m_context, BUTTON_R, 160ms, GameSettings::instance().BOX_CHANGE_DELAY0);
            pbf_move_right_joystick(m_context, 0, 128, 20, 105);
            pbf_move_right_joystick(m_context, 128, 255, 20, 105);
            pbf_move_right_joystick(m_context, 128, 255, 20, 105);
            pbf_move_right_joystick(m_context, 128, 255, 20, 105);
            m_babies_saved++;
            if (m_babies_saved >= m_max_keepers){
                m_stream.log("Max keepers reached. Stopping program...");
                return true;
            }
            break;
        case StatsHuntAction::Discard:
            m_stream.log("Releasing Pokemon...", COLOR_PURPLE);
            release(m_stream, m_context);
        }
    }

    pbf_move_right_joystick(m_context, 128, 0, 160ms, BOX_SCROLL_DELAY);
    pbf_move_right_joystick(m_context, 255, 128, 160ms, BOX_SCROLL_DELAY);
    return false;
}
bool EggAutonomousState::process_batch(){
    overworld_to_box(m_stream, m_context);
    if (process_party()){
        return true;
    }
    withdraw_egg_column();
    box_to_overworld(m_stream, m_context);
    return false;
}




void EggAutonomousState::fetch_egg(){
    if (m_eggs_in_column >= 5){
        process_error("FetchFullColumn", "Attempted to fetch an egg when column is full.");
    }

    //  Move to corner.
    m_context.wait_for_all_requests();
    m_stream.log("Attempting to fetch an egg.");
    {
        ShortDialogWatcher dialog;
        int ret = run_until<ProControllerContext>(
            m_stream, m_context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 125, 0);
            },
            {{dialog}}
        );
        if (ret >= 0){
            return;
        }
    }
    m_context.wait_for(std::chrono::milliseconds(200));

    m_stream.log("Getting off bike.");
    if (m_on_bike){
        m_shortcut.run(m_context, 100);
        m_context.wait_for_all_requests();
        m_on_bike = false;
    }

    m_stream.log("Going to daycare man.");
    {
        ShortDialogWatcher dialog;
        int ret = run_until<ProControllerContext>(
            m_stream, m_context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 255, 30, 0);
                pbf_move_left_joystick(context, 128, 0, 35, 0);
                pbf_move_left_joystick(context, 255, 128, 60, 125);
            },
            {{dialog}}
        );
        if (ret >= 0){
            return;
        }
    }

    //  Talk to daycare man.
    {
        ShortDialogWatcher dialog;
        int ret = run_until<ProControllerContext>(
            m_stream, m_context,
            [](ProControllerContext& context){
                pbf_press_button(context, BUTTON_ZL, 160ms, 9840ms);
            },
            {{dialog}}
        );
        if (ret < 0){
            process_error("DaycareMan", "Unable to find daycare man.");
        }
        m_stream.log("Found daycare man!");
    }

    {
        EggReceivedDetector received;
        run_until<ProControllerContext>(
            m_stream, m_context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_ZL, 500);
                pbf_mash_button(context, BUTTON_B, 500);
            },
            {{received}}
        );
        m_stats.m_fetch_attempts++;
        if (received.fetched()){
            m_eggs_in_column++;
            m_stats.m_fetch_success++;
            m_stream.log("Fetched an egg!", COLOR_BLUE);
        }else{
            m_stream.log("No egg fetched.", COLOR_ORANGE);
        }
        m_env.update_stats();
    }

    m_stream.log("Getting back on bike.");
    m_shortcut.run(m_context, 100);
    m_on_bike = true;
    pbf_move_left_joystick(m_context, 0, 255, 125, 0);
}
void EggAutonomousState::hatch_egg(){
    if (m_eggs_in_party == 0){
        process_error("HatchFullParty", "State Inconsistency: An egg started hatching when you have no eggs in your party.");
    }

    //  Hatch the egg.
    VideoSnapshot overworld = m_stream.video().snapshot();
//    overworld.save("test-0.png");
    {
        pbf_mash_button(m_context, BUTTON_B, 10 * TICKS_PER_SECOND);
        m_context.wait_for_all_requests();

        ShortDialogWatcher dialog;
        int ret = wait_until(
            m_stream, m_context, std::chrono::seconds(30),
            {{dialog}}
        );
        if (ret < 0){
            process_error("NoHatchEnd", "End of hatch not detected after 30 seconds.");
//            OperationFailedException::fire(m_console, "End of hatch not detected after 30 seconds.");
        }
        m_stream.log("Egg finished hatching.");
        m_stats.m_hatched++;
        m_env.update_stats();
        pbf_mash_button(m_context, BUTTON_B, 1 * TICKS_PER_SECOND);
    }

    //  Return to overworld.
    while (true){
        m_context.wait_for_all_requests();

        //  Wait for steady state and read it again.
        m_context.wait_for(std::chrono::milliseconds(200));
        ImageMatchWatcher matcher(overworld.frame, {0.10, 0.10, 0.80, 0.60}, 100);
        ShortDialogPromptDetector prompt(m_stream.overlay(), {0.50, 0.60, 0.30, 0.20}, COLOR_GREEN);
        int ret = wait_until(
            m_stream, m_context, std::chrono::seconds(30),
            {
                {matcher},
                {prompt},
            }
        );
        switch (ret){
        case 0:
            m_stream.log("Returned to overworld.");
            m_eggs_in_party--;
            return;
        case 1:
            m_stream.log("Detected prompt. Please turn off nicknaming.", COLOR_RED);
            m_stats.m_errors++;
            throw UserSetupError(m_stream.logger(), "Please turn off nicknaming.");
        default:
            m_stream.log("Failed to detect overworld after 30 seconds. Did day/night change?", COLOR_RED);
//            pbf_mash_button(context, BUTTON_ZL, 30 * TICKS_PER_SECOND);
            return;
        }
    }
}

void EggAutonomousState::hatch_rest_of_party(){
    m_stream.log("Hatching rest of party without fetching...");
    while (m_eggs_in_party > 0){
        dump();
        ShortDialogWatcher dialog;
        FrozenImageDetector frozen(COLOR_CYAN, {0, 0, 1, 0.5}, std::chrono::seconds(60), 20);
        int ret = run_until<ProControllerContext>(
            m_stream, m_context,
            [&](ProControllerContext& context){
                egg_spin(context, 8min);
            },
            {
                {dialog},
                {frozen},
            }
        );
        switch (ret){
        case 0:
            m_stream.log("Egg is hatching!");
            m_context.wait_for_all_requests();
            hatch_egg();
            break;
        case 1:
            process_error("FrozenScreen", "Frozen screen detected. Possible game crash.");
        default:
            process_error("NoHatch", "No hatch detected after 8 minutes of spinning.");
        }

    }
}
void EggAutonomousState::spin_until_fetch_or_hatch(){
    m_context.wait_for_all_requests();
    m_stream.log("Looking for more eggs...");
    ShortDialogWatcher dialog;
    int ret = run_until<ProControllerContext>(
        m_stream, m_context,
        [&](ProControllerContext& context){
            egg_spin(context, m_travel_time_per_fetch);
        },
        {{dialog}}
    );
    m_context.wait_for(std::chrono::milliseconds(200));
    if (ret < 0){
//        m_stream.log("Attempting to fetch an egg.");
        fetch_egg();
    }else{
//        m_stream.log("Egg is hatching!");
//        hatch_egg();
    }
}

bool EggAutonomousState::overworld_detect_and_run_state(){
    ShortDialogWatcher dialog;
    m_context.wait_for_all_requests();
    m_context.wait_for(std::chrono::milliseconds(200));
    dump();

    //  Egg is hatching. Handle that now.
    if (dialog.detect(m_stream.video().snapshot())){
        m_stream.log("Egg is hatching!");
        hatch_egg();
        return false;
    }

    //  Need more eggs.
    if (m_eggs_in_column < 5){
        spin_until_fetch_or_hatch();
        return false;
    }

    //  More eggs to hatch.
    if (m_eggs_in_party > 0){
        hatch_rest_of_party();
        return false;
    }

    //  Done with batch.
    return true;
}




}
}
}
