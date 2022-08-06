/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_IVCheckerReader_H
#define PokemonAutomation_PokemonSwSh_IVCheckerReader_H

#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;


class IVCheckerReaderScope{
public:
    IVCheckerReaderScope(VideoOverlay& overlay, Language language);

    IVCheckerReader::Results read(Logger& logger, const ImageViewRGB32& frame);

    std::vector<ImageViewRGB32> dump_images(const ImageViewRGB32& frame);

private:
    IVCheckerValue read(Logger& logger, const ImageViewRGB32& frame, const InferenceBoxScope& box);

private:
    Language m_language;
    InferenceBoxScope m_box0;
    InferenceBoxScope m_box1;
    InferenceBoxScope m_box2;
    InferenceBoxScope m_box3;
    InferenceBoxScope m_box4;
    InferenceBoxScope m_box5;
};



}
}
}
#endif
