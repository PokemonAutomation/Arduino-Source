/*  MMO Question Mark Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detect MMO question mark symbol.
 */

#ifndef PokemonAutomation_PokemonLA_MMOQuestionMarkDetector_H
#define PokemonAutomation_PokemonLA_MMOQuestionMarkDetector_H

#include <array>
#include <vector>
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{

class VideoOverlaySet;


namespace NintendoSwitch{
namespace PokemonLA{



class MMOQuestionMarkDetector{
public:
    MMOQuestionMarkDetector(Logger& logger);

    void make_overlays(VideoOverlaySet& items) const;

    // Detect the MMO question marks on the Hisui map when you leave village.
    // Return an array of bool, each bool is whether MMO appears on one of the 
    // wild region. The order of the bool is the same order as the game progession:
    // Fieldlands, Mirelands, Coastlands, Highlands, Icelands.
    std::array<bool, 5> detect_MMO_on_hisui_map(const ImageViewRGB32& frame);

    std::vector<ImagePixelBox> detect_MMOs_on_region_map(const ImageViewRGB32& frame);

private:
    Logger& m_logger;
};

// Show output of `MMOQuestionMarkDetector::detect_MMO_on_hisui_map()` to video overlay.
void add_hisui_MMO_detection_to_overlay(const std::array<bool, 5>& detection_result, VideoOverlaySet& items);


}
}
}
#endif
