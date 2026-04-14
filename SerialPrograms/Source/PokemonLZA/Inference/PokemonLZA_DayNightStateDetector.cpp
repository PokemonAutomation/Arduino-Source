#include "CommonFramework/ImageTools/ImageBoxes.h"

#include "CommonTools/Images/BinaryImage_FilterRgb32.h"

#include "PokemonLZA_DayNightStateDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


namespace{

// minimap crescent icon region (screen relative)
const ImageFloatBox MOON_BOX(
    0.075,
    0.025,
    0.040,
    0.050
    );


// color range of moon icon
const uint32_t MOON_COLOR_MIN = 0xffc0c0c0;
const uint32_t MOON_COLOR_MAX = 0xffffffff;


// ratio thresholds (resolution independent)
const double MIN_MOON_RATIO = 0.03;
const double MAX_MOON_RATIO = 0.40;


}



DayNightStateDetector::DayNightStateDetector(
    Color color,
    VideoOverlay* overlay
    )
    : StaticScreenDetector()
    , m_state(DayNightState::UNKNOWN)
{}


void DayNightStateDetector::make_overlays(
    VideoOverlaySet& items
    ) const{

    items.add(COLOR_RED, MOON_BOX);

}



bool DayNightStateDetector::detect(
    const ImageViewRGB32& screen
    ){

    ImageViewRGB32 roi =
        extract_box_reference(
            screen,
            MOON_BOX
            );


    if (!roi){

        m_state = DayNightState::UNKNOWN;

        return false;
    }


    PackedBinaryMatrix matrix =
        compress_rgb32_to_binary_range(
            roi,
            MOON_COLOR_MIN,
            MOON_COLOR_MAX
            );


    size_t matching_pixels = 0;

    for (size_t y = 0; y < matrix.height(); y++){
        for (size_t x = 0; x < matrix.width(); x++){

            if (matrix.get(x,y)){
                matching_pixels++;
            }

        }
    }


    const double moon_ratio =
        (double)matching_pixels /
        (double)(matrix.width() * matrix.height());


    const bool moon_detected =
        moon_ratio >= MIN_MOON_RATIO &&
        moon_ratio <= MAX_MOON_RATIO;


    m_state =
        moon_detected
            ? DayNightState::NIGHT
            : DayNightState::DAY;


    return true;
}



void DayNightStateDetector::reset_state(){

    m_state = DayNightState::UNKNOWN;

}



}
}
}