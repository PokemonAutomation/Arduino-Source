#ifndef BOXSHINYDETECTOR_H
#define BOXSHINYDETECTOR_H

#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"
#include "PokemonBDSP/Options/PokemonBDSP_EggHatchFilter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

class BoxGenderDetector{

public:
EggHatchGenderFilter identify_gender(LoggerQt& logger, VideoOverlay& overlay,const QImage& frame);

};
}
}
}
#endif
