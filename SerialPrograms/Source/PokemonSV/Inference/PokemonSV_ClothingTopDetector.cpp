/*  Clothing Top Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV_ClothingTopDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

//This detects that the gradient arrow is in the top slot of a clothing store menu
ClothingTopDetector::ClothingTopDetector(Color color)
    : m_arrow(color, GradientArrowType::RIGHT, { 0.009, 0.158, 0.061, 0.153 })
{}
void ClothingTopDetector::make_overlays(VideoOverlaySet& items) const {
    m_arrow.make_overlays(items);
}
bool ClothingTopDetector::detect(const ImageViewRGB32& screen) const {
    if (!m_arrow.detect(screen)) {
        return false;
    }
    return true;
}


}
}
}
