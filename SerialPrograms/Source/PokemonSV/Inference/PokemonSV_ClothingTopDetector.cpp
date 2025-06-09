/*  Clothing Top Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Tools/ErrorDumper.h"
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
void ClothingTopDetector::make_overlays(VideoOverlaySet& items) const{
    m_arrow.make_overlays(items);
}
bool ClothingTopDetector::detect(const ImageViewRGB32& screen){
    if (!m_arrow.detect(screen)){
        return false;
    }
    return true;
}


}
}
}
