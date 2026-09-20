/*  IV Judge Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_IvJudgeReader_H
#define PokemonAutomation_PokemonSV_IvJudgeReader_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Inference/Pokemon_IvJudgeReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{
using namespace Pokemon;


const IvJudgeReader& IV_READER();


class IvJudgeReaderScope{
public:
    IvJudgeReaderScope(VideoOverlay& overlay, Language language);

    IvJudgeReader::Results read(Logger& logger, const ImageViewRGB32& frame);

    std::vector<ImageViewRGB32> dump_images(const ImageViewRGB32& frame);

private:
    IvJudgeValue read(Logger& logger, const ImageViewRGB32& frame, const OverlayBoxScope& box);

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
