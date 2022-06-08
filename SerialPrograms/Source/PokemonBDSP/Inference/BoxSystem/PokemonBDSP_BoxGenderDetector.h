#ifndef BOXSHINYDETECTOR_H
#define BOXSHINYDETECTOR_H

#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"
#include "PokemonBDSP/Options/PokemonBDSP_EggHatchFilter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
using namespace Pokemon;

class BoxGenderDetector{

public:
EggHatchGenderFilter read_gender(LoggerQt& logger, VideoOverlay& overlay,const QImage& frame, QString name);

};
}
}
}
#endif
