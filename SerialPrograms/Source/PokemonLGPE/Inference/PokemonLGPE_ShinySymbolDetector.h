/*  Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonRSE_ShinyNumberDetector_H
#define PokemonAutomation_PokemonRSE_ShinyNumberDetector_H

#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

// After an in-game trade, the Pokemon's summary will appear.
// Red star for shiny. No star if not.
// Position is different when viewing in boxes. Do not use this for that.
class ShinySymbolDetector{
public:
    ShinySymbolDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const;
    bool read(Logger& logger, const ImageViewRGB32& frame);

private:
    ImageFloatBox m_box_star;
};



}
}
}
#endif
