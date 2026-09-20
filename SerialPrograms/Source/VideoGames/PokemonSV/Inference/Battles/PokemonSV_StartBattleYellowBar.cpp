/*  Start Battle Yellow Bar
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSV_StartBattleYellowBar.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



StartBattleYellowBarDetector::StartBattleYellowBarDetector(Color color)
    : m_color(color)
    , m_top(0.100000, 0.001000, 0.800000, 0.004000)
    , m_bot(0.100000, 0.995000, 0.800000, 0.004000)
    , m_mid(0.1, 0.1, 0.8, 0.8)
{}
void StartBattleYellowBarDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_top);
    items.add(m_color, m_bot);
}

bool StartBattleYellowBarDetector::detect(const ImageViewRGB32& screen){
    ImageStats top = image_stats(extract_box_reference(screen, m_top));
    ImageStats bot = image_stats(extract_box_reference(screen, m_bot));
//    cout << "top = " << top.average << top.stddev << endl;
//    cout << "bot = " << bot.average << top.stddev << endl;
    if (!is_solid(top, {0.554825, 0.445175, 0.})){
        return false;
    }
    if (!is_solid(bot, {0.554825, 0.445175, 0.})){
        return false;
    }

    if (euclidean_distance(top.average, bot.average) > 10){
        return false;
    }

    ImageStats mid = image_stats(extract_box_reference(screen, m_mid));
//    cout << "mid = " << mid.average << mid.stddev << endl;
    if (mid.stddev.sum() < 10){
        return false;
    }

    return true;
}




void StartBattleYellowBarWatcher::make_overlays(VideoOverlaySet& items) const{
    StartBattleYellowBarDetector::make_overlays(items);
}
bool StartBattleYellowBarWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    if (!detect(frame)){
        m_start_detection = WallClock::min();
        return false;
    }

    //  Not long enough.
    if (m_start_detection + std::chrono::milliseconds(100) > timestamp){
        return false;
    }

    m_last_detected = timestamp;
//    cout << "Detected yellow bar." << endl;
    return false;
}




}
}
}
