/*  Battle Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "PokemonLGPE_BattleArrowDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

const ImageMatch::ExactImageMatcher& BATTLE_ARROW(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonLGPE/BattleArrow.png");
    return matcher;
}

BattleArrowDetector::BattleArrowDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_box(box)
{}
void BattleArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool BattleArrowDetector::detect(const ImageViewRGB32& screen){
    using namespace Kernels::Waterfill;

    ImageViewRGB32 region = extract_box_reference(screen, m_box);

    const ImageMatch::ExactImageMatcher& matcher = BATTLE_ARROW();

    auto matrix = compress_rgb32_to_binary_range(region, 0xffc0c0c0, 0xffffffff);
    auto session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(100);
    WaterfillObject object;

    //static int c = 0;
    while (iter->find_next(object, false)){
        double aspect_ratio = object.aspect_ratio();
        if (aspect_ratio < 1.0 || aspect_ratio > 1.3){
            continue;
        }
        ImageViewRGB32 cropped = extract_box_reference(region, object);
        //cropped.save("test-object-" + std::to_string(c++) + ".png");
        double rmsd = matcher.rmsd(cropped);
//        cout << rmsd << endl;
        if (rmsd < 116){
            return true;
        }
    }

    return false;
}


bool BattleArrowWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    if (!detect(frame)){
        return false;
    }
    m_detections.emplace_back(timestamp);

    WallClock threshold = timestamp - std::chrono::seconds(5);
    while (!m_detections.empty() && m_detections.front() < threshold){
        m_detections.pop_front();
    }

//    cout << "m_detections = " << m_detections.size() << endl;

    return m_detections.size() >= 3;
}





}
}
}
