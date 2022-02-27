/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_IVCheckerReader_H
#define PokemonAutomation_PokemonBDSP_IVCheckerReader_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
using namespace Pokemon;


class IVCheckerReaderScope{
public:
    IVCheckerReaderScope(VideoOverlay& overlay, Language language);

    IVCheckerReader::Results read(LoggerQt& logger, const QImage& frame);

    std::vector<QImage> dump_images(const QImage& frame);

private:
    IVCheckerValue read(LoggerQt& logger, const QImage& frame, const InferenceBoxScope& box);

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
