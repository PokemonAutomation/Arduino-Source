#include "PokemonLZA_DayNightStateDetector.h"

#include "CommonFramework/ImageTools/ImageStats.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

DayNightStateDetector::DayNightStateDetector()
    :
    m_box(0.40, 0.80, 0.45, 0.45),
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


    bool night_color =
        r < 120 &&
        b > 70 &&
        luminance < 140;


    if (night_color){
        m_state = DayNightState::NIGHT;
    }
    else{
        m_state = DayNightState::DAY;
    }

    return true;
}

}
}
}