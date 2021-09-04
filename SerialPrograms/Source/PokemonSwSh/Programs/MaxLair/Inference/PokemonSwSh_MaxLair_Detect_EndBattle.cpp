/*  Max Lair Detect Battle Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonSwSh_MaxLair_Detect_EndBattle.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


PokemonCaughtMenuDetector::PokemonCaughtMenuDetector(VideoOverlay& overlay)
    : m_top_white(overlay, 0.550, 0.020, 0.400, 0.020, Qt::magenta)
    , m_caught_left(overlay, 0.500, 0.080, 0.050, 0.070, Qt::magenta)
    , m_caught_right(overlay, 0.930, 0.080, 0.050, 0.070, Qt::magenta)
    , m_middle_pink(overlay, 0.930, 0.300, 0.050, 0.200, Qt::magenta)
    , m_bottom_white(overlay, 0.550, 0.900, 0.400, 0.020, Qt::magenta)
    , m_bottom_black(overlay, 0.100, 0.970, 0.700, 0.020, Qt::magenta)
    , m_bottom_options(overlay, 0.920, 0.970, 0.070, 0.020, Qt::magenta)
{}
bool PokemonCaughtMenuDetector::on_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}


bool PokemonCaughtMenuDetector::detect(const QImage& screen){
    ImageStats top_white = image_stats(extract_box(screen, m_top_white));
//    cout << top_white.average << ", " << top_white.stddev << endl;
    if (!is_solid(top_white, {0.316068, 0.341966, 0.341966})){
        return false;
    }
    ImageStats caught_left = image_stats(extract_box(screen, m_caught_left));
    if (!is_black(caught_left)){
        return false;
    }
    ImageStats caught_right = image_stats(extract_box(screen, m_caught_right));
    if (!is_black(caught_right)){
        return false;
    }
    if (euclidean_distance(caught_left.average, caught_right.average) > 10){
        return false;
    }
    ImageStats middle_pink = image_stats(extract_box(screen, m_middle_pink));
    if (!is_solid(middle_pink, {0.485975, 0.0980567, 0.415969})){
        return false;
    }
    ImageStats bottom_white = image_stats(extract_box(screen, m_bottom_white));
//    cout << bottom_white.average << ", " << bottom_white.stddev << endl;
    if (!is_solid(bottom_white, {0.331264, 0.332167, 0.336569}, 0.1, 20)){
        return false;
    }
    ImageStats bottom_black = image_stats(extract_box(screen, m_bottom_black));
    if (!is_black(bottom_black)){
        return false;
    }
    ImageStats bottom_options = image_stats(extract_box(screen, m_bottom_options));
    if (bottom_options.stddev.sum() < 30){
        return false;
    }

    return true;
}


}
}
}
}
