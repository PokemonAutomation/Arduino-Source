/*  Blueberry Points Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "PokemonSV_BlueberryPointsReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


int read_BP_from_image(Logger& logger, const ImageViewRGB32& screen){
    // Normalized coordinates for the upper-right balance on a 16:9 quest panel.
    // Keep the right edge at 0.954: the old 0.957 edge included the left stroke
    // of the B in "BP", making 6750 read as 67501. See the full-frame reference
    // UnitTestResources/PokemonSV/BlueberryPoints/6750.png and its README.
    const ImageFloatBox BP_NUMBER_BOX(0.866, 0.019, 0.088, 0.041);
    // Isolate the near-white digits from the blue panel background. These are
    // the existing quest-panel text thresholds, inclusive on all RGB channels.
    const uint32_t BP_TEXT_MIN = combine_rgb(198, 198, 198);
    const uint32_t BP_TEXT_MAX = combine_rgb(255, 255, 255);
    ImageRGB32 number = to_blackwhite_rgb32_range(
        extract_box_reference(screen, BP_NUMBER_BOX), true, BP_TEXT_MIN, BP_TEXT_MAX
    );
    return OCR::read_number(logger, number);
}


}
}
}
