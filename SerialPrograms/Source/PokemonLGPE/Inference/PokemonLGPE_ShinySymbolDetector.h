/*  Shiny Symbol Detector
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
namespace PokemonLGPE{

//Check for the red shiny star on a Pokemon's summary from the Party/Box menu.
//This does not work for the summary that appears after a catch.
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
