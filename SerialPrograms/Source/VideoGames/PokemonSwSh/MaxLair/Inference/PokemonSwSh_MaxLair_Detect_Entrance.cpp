/*  Max Lair Detect Entrance
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSwSh_MaxLair_Detect_Entrance.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


EntranceDetector::EntranceDetector(const ImageViewRGB32& entrance_screen)
    : VisualInferenceCallback("EntranceDetector")
    , m_box0(0.020, 0.020, 0.500, 0.750)
    , m_watch_box(extract_box_reference(entrance_screen, m_box0))
{}
void EntranceDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_DARKGREEN, m_box0);
}
bool EntranceDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return detect(frame);
}

bool EntranceDetector::detect(const ImageViewRGB32& screen){
    if (!screen){
        return false;
    }

    ImageRGB32 copy;

    ImageViewRGB32 image = extract_box_reference(screen, m_box0);
    if (image.width() != (size_t)m_watch_box.width() || image.height() != (size_t)m_watch_box.height()){
        copy = image.scale_to(m_watch_box.width(), m_watch_box.height());
        image = copy;
    }

    double diff = ImageMatch::pixel_RMSD(m_watch_box, image);
//    cout << diff << endl;

    return diff < 20;
}



}
}
}
}
