/*  Box Nature Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BoxNatureDetector_H
#define PokemonAutomation_PokemonBDSP_BoxNatureDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Inference/Pokemon_NatureReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
using namespace Pokemon;

const NatureReader& NATURE_READER();

class BoxNatureDetector{
public:
    BoxNatureDetector(VideoOverlay& overlay, Language language);

    NatureReader::Results read(Logger& logger, const ImageViewRGB32& frame);

private:
    NatureCheckerValue read(Logger& logger, const ImageViewRGB32& frame, const OverlayBoxScope& box);

private:
    Language m_language;
    OverlayBoxScope m_nature;
};



}
}
}
#endif
