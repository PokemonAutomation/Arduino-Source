/*  Shiny Number Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_ShinyNumberDetector_H
#define PokemonAutomation_PokemonRSE_ShinyNumberDetector_H

#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

// In the summary screen, the dex number will be yellow if a shiny, white if not.
// Additionally, the background behind the sprite will be white if shiny, grey if not.
// Number is easier to check as the background is scan lines.
class ShinyNumberDetector{
public:
    ShinyNumberDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const;
    bool read(Logger& logger, const ImageViewRGB32& frame);

private:
    ImageFloatBox m_box_number;
};



}
}
}
#endif
