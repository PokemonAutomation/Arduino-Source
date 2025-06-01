/*  Encounter Watcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Images/ImageFilter.h"
#include "PokemonSV_EncounterWatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


EncounterWatcher::EncounterWatcher(VideoStream& stream, Color color)
    : VisualInferenceCallback("EncounterWatcher (video)")
    , AudioInferenceCallback("EncounterWatcher (audio)")
    , m_battle_menu(color)
    , m_shiny_sound(stream.logger(), [](float){ return true; })
{}
void EncounterWatcher::make_overlays(VideoOverlaySet& items) const{
    m_battle_menu.make_overlays(items);
}
bool EncounterWatcher::process_frame(const VideoSnapshot& frame){
    if (!frame){
        return false;
    }

    {
        std::lock_guard<std::mutex> lg(m_lock);
        //  Clear old history.
        while (!m_history.empty() && m_history.front().timestamp + std::chrono::milliseconds(2000) < frame.timestamp){
            m_history.pop_front();
        }

        //  Add current frame if it has been long enough since the previous.
        if (m_history.empty() || m_history.back().timestamp + std::chrono::milliseconds(250) < frame.timestamp){
            m_history.push_back(frame);
        }
    }

    return m_battle_menu.process_frame(frame, frame.timestamp);
}
bool EncounterWatcher::process_spectrums(
    const std::vector<AudioSpectrum>& new_spectrums,
    AudioFeed& audio_feed
){
    bool detected = m_shiny_sound.process_spectrums(new_spectrums, audio_feed);
    if (!detected){
        return false;
    }
//    cout << "detected" << endl;

    WallClock threshold = current_time() - std::chrono::seconds(1);

    //  Find the brightest frame.
    std::lock_guard<std::mutex> lg(m_lock);

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
            m_best_snapshot = frame;
        }
    }

    return false;
}




}
}
}
