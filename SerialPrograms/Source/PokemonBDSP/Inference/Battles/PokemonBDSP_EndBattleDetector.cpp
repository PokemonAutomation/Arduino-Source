/*  End Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "PokemonBDSP_EndBattleDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



#if 0
EndBattleWatcher::EndBattleWatcher(const ImageFloatBox& box, Color color)
    : VisualInferenceCallback("EndBattleWatcher")
    , m_color(color)
    , m_box(box)
    , m_has_been_black(false)
{}
void EndBattleWatcher::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool EndBattleWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return battle_is_over(frame);
}
bool EndBattleWatcher::battle_is_over(const ImageViewRGB32& frame){
    ImageViewRGB32 image = extract_box_reference(frame, m_box);
    ImageStats stats = image_stats(image);
    if (is_black(stats)){
        m_has_been_black = true;
        return false;
    }
    if (!m_has_been_black){
        return false;
    }
//    cout << stats.stddev.sum() << endl;
    if (stats.stddev.sum() < 20){
        return false;
    }
    return true;
}
#endif



}
}
}
