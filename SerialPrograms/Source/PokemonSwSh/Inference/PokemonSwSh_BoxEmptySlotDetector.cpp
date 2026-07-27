/*  Box Empty Slot Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh_BoxEmptySlotDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{




BoxEmptySlotDetector::BoxEmptySlotDetector(SlotLocation side, uint8_t row, uint8_t column, Color color)
: m_color(color){
    if (side == SlotLocation::PARTY){
        if (row > 5){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "INVALID party row for BoxEmptySlotDetector");
        }
        m_box = ImageFloatBox(0.087527, 0.1255 * row + 0.192607, 0.079869, 0.089494);
    }else if (side == SlotLocation::BOX){
        if (row > 4 || column > 5){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "INVALID box row/column for BoxEmptySlotDetector");
        }
        m_box = ImageFloatBox(0.0705 * column + 0.265, 0.1255 * row + 0.260700, 0.043, 0.023346);
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "INVALID SlotLocation for BoxEmptySlotDetector");
    }
}

void BoxEmptySlotDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool BoxEmptySlotDetector::detect(const ImageViewRGB32& screen){
    ImageStats stats = image_stats(extract_box_reference(screen, m_box));

    const double max_stddev_sum = 50;
    const double stddev = stats.stddev.sum();
    // cout << "BoxEmptySlotDetector::detect: stddev sum " << stddev << " vs max " << max_stddev_sum << endl;

    // if the stddev.sum() is low, it's likely an empty slot
    return stddev <= max_stddev_sum;
}






}
}
}
