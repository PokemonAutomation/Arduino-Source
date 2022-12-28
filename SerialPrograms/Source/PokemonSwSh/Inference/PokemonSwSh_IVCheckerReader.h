/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_IVCheckerReader_H
#define PokemonAutomation_PokemonSwSh_IVCheckerReader_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;


const IVCheckerReader& IV_READER();


class IVCheckerReaderScope{
public:
    IVCheckerReaderScope(VideoOverlay& overlay, Language language);

    IVCheckerReader::Results read(Logger& logger, const ImageViewRGB32& frame);

    std::vector<ImageViewRGB32> dump_images(const ImageViewRGB32& frame);

private:
    IVCheckerValue read(Logger& logger, const ImageViewRGB32& frame, const OverlayBoxScope& box);

private:
    Language m_language;
    OverlayBoxScope m_box0;
    OverlayBoxScope m_box1;
    OverlayBoxScope m_box2;
    OverlayBoxScope m_box3;
    OverlayBoxScope m_box4;
    OverlayBoxScope m_box5;
};



}
}
}
#endif
