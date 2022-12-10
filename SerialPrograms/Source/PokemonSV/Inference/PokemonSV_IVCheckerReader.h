/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_IVCheckerReader_H
#define PokemonAutomation_PokemonSV_IVCheckerReader_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{
using namespace Pokemon;


class IVCheckerReaderScope{
public:
    IVCheckerReaderScope(VideoOverlay& overlay, Language language);

    IVCheckerReader::Results read(Logger& logger, const ImageViewRGB32& frame);

    std::vector<ImageViewRGB32> dump_images(const ImageViewRGB32& frame);

private:
    IVCheckerValue read(Logger& logger, const ImageViewRGB32& frame, const OverlayBoxScope& box);

private:
    Language m_language;
    OverlayBoxScope m_box_hp;
    OverlayBoxScope m_box_attack;
    OverlayBoxScope m_box_defense;
    OverlayBoxScope m_box_spatk;
    OverlayBoxScope m_box_spdef;
    OverlayBoxScope m_box_speed;
};



}
}
}
#endif
