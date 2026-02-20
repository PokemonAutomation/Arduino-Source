/*  Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_ShinySymbolDetector_H
#define PokemonAutomation_PokemonFRLG_ShinySymbolDetector_H

#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

// Shinies have a bright yellow star symbol in their summary
// additionally, the border in the screen is blue instead of purple
class ShinySymbolDetector{
public:
    ShinySymbolDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const;
    bool read(Logger& logger, const ImageViewRGB32& frame);

private:
    ImageFloatBox m_box_symbol;
};



}
}
}
#endif
