/*  Max Lair Detect Battle Menu
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSwSh_MaxLair_Detect_EndBattle.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


PokemonCaughtMenuDetector::PokemonCaughtMenuDetector()
    : VisualInferenceCallback("PokemonCaughtMenuDetector")
    , m_top_white(0.550, 0.020, 0.400, 0.020)
    , m_caught_left(0.500, 0.080, 0.050, 0.070)
    , m_caught_right(0.930, 0.080, 0.050, 0.070)
    , m_middle_pink(0.930, 0.300, 0.050, 0.200)
    , m_bottom_white(0.550, 0.900, 0.400, 0.020)
    , m_bottom_black(0.100, 0.970, 0.600, 0.020)
    , m_bottom_options(0.920, 0.970, 0.070, 0.020)
{}
void PokemonCaughtMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_MAGENTA, m_top_white);
    items.add(COLOR_MAGENTA, m_caught_left);
    items.add(COLOR_MAGENTA, m_caught_right);
    items.add(COLOR_MAGENTA, m_middle_pink);
    items.add(COLOR_MAGENTA, m_bottom_white);
    items.add(COLOR_MAGENTA, m_bottom_black);
    items.add(COLOR_MAGENTA, m_bottom_options);
}
bool PokemonCaughtMenuDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return detect(frame);
}


bool PokemonCaughtMenuDetector::detect(const ImageViewRGB32& screen){
    ImageStats top_white = image_stats(extract_box_reference(screen, m_top_white));
//    cout << top_white.average << ", " << top_white.stddev << endl;
    if (!is_solid(top_white, {0.316068, 0.341966, 0.341966})){
//        cout << "Failed: m_top_white" << endl;
        return false;
    }
    ImageStats caught_left = image_stats(extract_box_reference(screen, m_caught_left));
    if (!is_black(caught_left)){
//        cout << "Failed: m_caught_left" << endl;
        return false;
    }
    ImageStats caught_right = image_stats(extract_box_reference(screen, m_caught_right));
    if (!is_black(caught_right)){
//        cout << "Failed: m_caught_right" << endl;
        return false;
    }
    if (euclidean_distance(caught_left.average, caught_right.average) > 10){
//        cout << "Bad left/right distance." << endl;
        return false;
    }
    ImageStats middle_pink = image_stats(extract_box_reference(screen, m_middle_pink));
    if (!is_solid(middle_pink, {0.485975, 0.0980567, 0.415969})){
//        cout << middle_pink.average << ", " << middle_pink.stddev << endl;
//        cout << "Failed: m_middle_pink" << endl;
        return false;
    }
    ImageStats bottom_white = image_stats(extract_box_reference(screen, m_bottom_white));
//    cout << bottom_white.average << ", " << bottom_white.stddev << endl;
    if (!is_solid(bottom_white, {0.331264, 0.332167, 0.336569}, 0.1, 20)){
        return false;
    }
    ImageStats bottom_black = image_stats(extract_box_reference(screen, m_bottom_black));
    if (!is_black(bottom_black)){
        return false;
    }
    ImageStats bottom_options = image_stats(extract_box_reference(screen, m_bottom_options));
    if (bottom_options.stddev.sum() < 30){
        return false;
    }

    return true;
}



size_t count_catches(VideoOverlay& overlay, const ImageViewRGB32& screen){
    OverlayBoxScope box0(overlay, {0.780, 0.400 + 0*0.133, 0.030, 0.030}, COLOR_BLUE);
    OverlayBoxScope box1(overlay, {0.780, 0.400 + 1*0.133, 0.030, 0.030}, COLOR_BLUE);
    OverlayBoxScope box2(overlay, {0.780, 0.400 + 2*0.133, 0.030, 0.030}, COLOR_BLUE);
    OverlayBoxScope box3(overlay, {0.780, 0.400 + 3*0.133, 0.030, 0.030}, COLOR_BLUE);

    size_t count = 0;
    if (is_black(image_stats(extract_box_reference(screen, box0)))){
        count++;
    }
    if (is_white(image_stats(extract_box_reference(screen, box1)))){
        count++;
    }
    if (is_white(image_stats(extract_box_reference(screen, box2)))){
        count++;
    }
    if (is_white(image_stats(extract_box_reference(screen, box3)))){
        count++;
    }

    return count;
}

















}
}
}
}
