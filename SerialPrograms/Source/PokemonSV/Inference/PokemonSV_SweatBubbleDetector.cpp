/*  Sweat Bubble Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "PokemonSV_SweatBubbleDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


const ImageMatch::ExactImageMatcher& SWEAT_BUBBLE(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSV/SweatBubble.png");
    return matcher;
}



SweatBubbleDetector::SweatBubbleDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_box(box)
{}
void SweatBubbleDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool SweatBubbleDetector::detect(const ImageViewRGB32& screen){
    using namespace Kernels::Waterfill;

    ImageViewRGB32 region = extract_box_reference(screen, m_box);

    const ImageMatch::ExactImageMatcher& matcher = SWEAT_BUBBLE();

    auto matrix = compress_rgb32_to_binary_range(region, 0xffc0c0c0, 0xffffffff);
    auto session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(100);
    WaterfillObject object;

//    static int c = 0;
    while (iter->find_next(object, false)){
        double aspect_ratio = object.aspect_ratio();
        if (aspect_ratio < 1.0 || aspect_ratio > 1.3){
            continue;
        }
        ImageViewRGB32 cropped = extract_box_reference(region, object);
//        cropped.save("test-object-" + std::to_string(c++) + ".png");
        double rmsd = matcher.rmsd(cropped);
//        cout << rmsd << endl;
        if (rmsd < 80){
            return true;
        }
    }

    return false;
}




}
}
}
