/*  IV Judge Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_IvJudgeReader_H
#define PokemonAutomation_PokemonBDSP_IvJudgeReader_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Inference/Pokemon_IvJudgeReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
using namespace Pokemon;


class IvJudgeReaderScope{
public:
    IvJudgeReaderScope(VideoOverlay& overlay, Language language);
    
    IvJudgeReader::Results read(Logger& logger, const ImageViewRGB32& frame);

    std::vector<ImageViewRGB32> dump_images(const ImageViewRGB32& frame);

private:
    IvJudgeValue read(Logger& logger, const ImageViewRGB32& frame, const OverlayBoxScope& box);

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
