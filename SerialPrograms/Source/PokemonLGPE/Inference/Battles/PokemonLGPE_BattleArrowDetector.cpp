/*  Battle Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonLGPE_BattleArrowDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{


FixedLimitVector<ImageMatch::WaterfillTemplateMatcher> make_battle_arrow_template(){
    static constexpr size_t TOTAL_IMAGES = 8;
    FixedLimitVector<ImageMatch::WaterfillTemplateMatcher> ret(TOTAL_IMAGES);
    for (size_t c = 0; c < TOTAL_IMAGES; c++){
        ret.emplace_back(
            ("PokemonLGPE/BattleArrow-" + std::to_string(c) + ".png").c_str(),
            Color(0xffc0c0c0), Color(0xffffffff),
            100
        );
    }
    return ret;
}


const FixedLimitVector<ImageMatch::WaterfillTemplateMatcher>& BATTLE_ARROWS(){
    static FixedLimitVector<ImageMatch::WaterfillTemplateMatcher> matcher = make_battle_arrow_template();
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

//    cout << "------------" << endl;
    auto matrix = compress_rgb32_to_binary_range(region, 0xffc0c0c0, 0xffffffff);
    auto session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(100);
    WaterfillObject object;

    //static int c = 0;
    while (iter->find_next(object, false)){
        double aspect_ratio = object.aspect_ratio();
        if (aspect_ratio < 0.8 || aspect_ratio > 1.3){
            continue;
        }
        ImageViewRGB32 cropped = extract_box_reference(region, object);
//        cropped.save("test-object-" + std::to_string(c++) + ".png");
//        cropped.save("testT.png");
        for (const ImageMatch::WaterfillTemplateMatcher& matcher : BATTLE_ARROWS()){
 //           matcher.image_template().save("test0.png");
            double rmsd = matcher.rmsd(Resolution(screen.width(), screen.height()), cropped);
//            cout << rmsd << endl;
            if (rmsd < 60){
                return true;
            }
        }
    }

    return false;
}


#if 0
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
#endif




}
}
}
