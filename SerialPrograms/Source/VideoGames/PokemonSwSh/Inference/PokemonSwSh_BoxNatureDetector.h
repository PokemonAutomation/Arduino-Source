/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BoxNatureDetector_H
#define PokemonAutomation_PokemonSwSh_BoxNatureDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Inference/Pokemon_NatureReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;

enum class NaturePlusMinus{
    NEUTRAL = 0,
    PLUS,
    MINUS,
};

class BoxNatureDetector{
public:
    BoxNatureDetector(VideoOverlay& overlay);

    NatureReader::Results read(Logger& logger, const ImageViewRGB32& frame);

private:
    NaturePlusMinus read(Logger& logger, const ImageViewRGB32& frame, const ImageFloatBox& box);

private:
    OverlayBoxScope m_box_atk;
    OverlayBoxScope m_box_def;
    OverlayBoxScope m_box_spatk;
    OverlayBoxScope m_box_spdef;
    OverlayBoxScope m_box_spd;
};



}
}
}
#endif
