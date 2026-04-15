#include "PokemonLZA_DayNightStateDetector.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include <iostream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

DayNightStateDetector::DayNightStateDetector(VideoOverlay* overlay)
    :
    m_box(0.02, 0.50, 0.08, 0.10),
    m_state(DayNightState::DAY)
{}


void DayNightStateDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_GREEN, m_box);
}

bool DayNightStateDetector::detect(const ImageViewRGB32& screen){

    ImageStats stats =
        image_stats(extract_box_reference(screen, m_box));

    double r = stats.average.r;
    double g = stats.average.g;
    double b = stats.average.b;

    double luminance =
        0.299*r +
        0.587*g +
        0.114*b;
   
    std::cout << "RGB: (" << r << ", " << g << ", " << b << ") Luminance: " << luminance << std::endl;

    bool night_color =
        r < 10 &&
        b > 20 &&
        luminance < 20;

    bool day_color =
        r > 100 && r < 180 &&
        g > 140 &&
        b > 160 &&
        luminance > 140;

    if (night_color) {
        m_state = DayNightState::NIGHT;
    }
    else if (day_color) {
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