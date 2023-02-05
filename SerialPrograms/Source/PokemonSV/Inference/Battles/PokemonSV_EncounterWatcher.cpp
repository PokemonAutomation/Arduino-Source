/*  Encounter Watcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/ImageFilter.h"
#include "PokemonSV_EncounterWatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


EncounterWatcher::EncounterWatcher(ConsoleHandle& console, Color color)
    : VisualInferenceCallback("EncounterWatcher (video)")
    , AudioInferenceCallback("EncounterWatcher (audio)")
    , m_battle_menu(color)
    , m_shiny_sound(console, [](float){ return true; })
{}
void EncounterWatcher::make_overlays(VideoOverlaySet& items) const{
    m_battle_menu.make_overlays(items);
}
bool EncounterWatcher::process_frame(const VideoSnapshot& frame){
    if (!frame){
        return false;
    }

    //  Clear old history.
    while (!m_history.empty() && m_history.front().timestamp + std::chrono::milliseconds(2000) < frame.timestamp){
        m_history.pop_front();
    }

    //  Add current frame if it has been long enough since the previous.
    if (m_history.empty() || m_history.back().timestamp + std::chrono::milliseconds(250) < frame.timestamp){
        m_history.push_back(frame);
    }

    return m_battle_menu.process_frame(frame, frame.timestamp);
}
bool EncounterWatcher::process_spectrums(
    const std::vector<AudioSpectrum>& newSpectrums,
    AudioFeed& audioFeed
){
    bool detected = m_shiny_sound.process_spectrums(newSpectrums, audioFeed);
    if (!detected){
        return false;
    }
//    cout << "detected" << endl;

    //  Find the brighest frame.
    double best_bright_portion = 0;
    for (const VideoSnapshot& frame : m_history){
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
