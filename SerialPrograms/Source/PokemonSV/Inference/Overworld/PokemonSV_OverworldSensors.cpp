/*  Overworld Sensors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonSV_OverworldSensors.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



ShinySoundTracker::ShinySoundTracker(Logger& logger)
    : ShinySoundDetector(logger, [this](float error){
//        cout << "New: Shiny Sound" << endl;
        on_shiny_detected(error);
        return false;
    })
    , VisualInferenceCallback("ShinyScreenshotTracker")
    , m_last_detection(WallClock::min())
    , m_last_error(1.0)
{}
bool ShinySoundTracker::last_detection(
    WallClock& timestamp, float& error,
    VideoSnapshot& screenshot
) const{
    ReadSpinLock lg(m_lock);
    if (m_last_detection == WallClock::min()){
        return false;
    }
    timestamp = m_last_detection;
    error = m_last_error;
    screenshot = m_last_screenshot;
    return true;
}
void ShinySoundTracker::on_shiny_detected(float error){
    WallClock now = current_time();
    WallClock threshold = now - std::chrono::seconds(1);

    //  Find the brightest frame.
    ReadSpinLock lg(m_lock);

    VideoSnapshot best_snapshot;
    double best_bright_portion = 0;

    for (const VideoSnapshot& frame : m_history){
        if (frame.timestamp >= threshold){
            break;
        }
        size_t bright_pixels;
        filter_rgb32_range(bright_pixels, frame, 0xffe0e000, 0xffffffff, Color(0xff000000), false);
        double bright_portion = bright_pixels / (double)(frame->width() * frame->height());
        if (best_bright_portion < bright_portion){
            best_bright_portion = bright_portion;
            best_snapshot = frame;
        }
    }

    m_last_screenshot = std::move(best_snapshot);
    m_last_detection = now;
    m_last_error = error;
}
bool ShinySoundTracker::process_frame(const VideoSnapshot& frame){
    if (!frame){
        return false;
    }

    WriteSpinLock lg(m_lock);

    //  Clear old history.
    while (!m_history.empty() && m_history.front().timestamp + std::chrono::milliseconds(2000) < frame.timestamp){
        m_history.pop_front();
    }

    //  Add current frame if it has been long enough since the previous.
    if (m_history.empty() || m_history.back().timestamp + std::chrono::milliseconds(250) < frame.timestamp){
        m_history.push_back(frame);
    }

    return false;
}











OverworldBattleTracker::OverworldBattleTracker(Logger& logger, OverworldSensors& sensors)
    : m_logger(logger)
    , m_sensors(sensors)
    , m_start_time(current_time())
    , m_last_known_in_battle(false)
{}
auto OverworldBattleTracker::report_battle_menu_detected() -> Detection{
    bool is_shiny = false;
    WallClock last_shiny_sound;
    float last_shiny_error;
    VideoSnapshot last_shiny_screenshot;
    if (m_sensors.shiny_sound.last_detection(last_shiny_sound, last_shiny_error, last_shiny_screenshot)){
        is_shiny = m_start_time < last_shiny_sound;
    }

    bool start_battle = m_start_time < m_sensors.start_battle.last_detected();

    if (is_shiny && !start_battle){
        m_logger.log("Detected a shiny without a start of battle.", COLOR_RED);
    }
    if (!m_last_known_in_battle && !start_battle){
        m_logger.log("Detected a battle menu without a start of battle.", COLOR_RED);
    }

    if (is_shiny){
        return Detection::NEW_SHINY_BATTLE;
    }
    if (start_battle){
        return Detection::NEW_BATTLE;
    }
    return Detection::CONTINUE_BATTLE;
}
auto OverworldBattleTracker::report_battle_menu_detected(VideoSnapshot& shiny_screenshot, float& shiny_error) -> Detection{
    bool is_shiny = false;
    WallClock last_shiny_sound;
    if (m_sensors.shiny_sound.last_detection(last_shiny_sound, shiny_error, shiny_screenshot)){
        is_shiny = m_start_time < last_shiny_sound;
    }

    bool start_battle = m_start_time < m_sensors.start_battle.last_detected();

    if (is_shiny && !start_battle){
        m_logger.log("Detected a shiny without a start of battle.", COLOR_RED);
    }
    if (!m_last_known_in_battle && !start_battle){
        m_logger.log("Detected a battle menu without a start of battle.", COLOR_RED);
    }

    if (is_shiny){
        return Detection::NEW_SHINY_BATTLE;
    }
    if (start_battle){
        return Detection::NEW_BATTLE;
    }
    return Detection::CONTINUE_BATTLE;
}






















}
}
}
