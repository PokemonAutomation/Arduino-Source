#include "PokemonLZA_DayNightStateDetector.h"

#include "CommonFramework/ImageTools/ImageStats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


DayNightStateDetector::DayNightStateDetector(VideoOverlay* overlay)
    :
    // sample terrain area after zooming map fully in and hiding icons
    // screen-relative coordinates (0–1)
    m_box(0.30, 0.55, 0.15, 0.18),
    m_state(DayNightState::DAY)
{}


void DayNightStateDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_GREEN, m_box);
}


bool DayNightStateDetector::detect(const ImageViewRGB32& screen){

    ImageStats stats =
        image_stats(extract_box_reference(screen, m_box));


    // robust against brightness differences between capture cards
    const double blue_ratio =
        stats.average.b /
        (stats.average.r + stats.average.g + stats.average.b);


    /*
        Empirically stable separation:

        DAY   ≈ 0.32 – 0.35
        NIGHT ≈ 0.40 – 0.46

        Threshold chosen conservatively to survive:
        - HDMI color variance
        - OBS color space changes
        - capture card gamma shifts
    */

    if (blue_ratio > 0.36){
        m_state = DayNightState::NIGHT;
    }
    else{
        m_state = DayNightState::DAY;
    }

    return true;
}


DayNightState DayNightStateDetector::state() const{
    return m_state;
}


}
}
}