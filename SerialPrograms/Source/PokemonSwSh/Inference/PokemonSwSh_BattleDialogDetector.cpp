/*  Battle Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleDialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



bool is_dialog_grey(const ImageStats& stats){
//    cout << "stddev = " << stats.stddev.sum() << endl;
//    cout << "average = " << stats.average << endl;

    if (stats.average.sum() > 200 || stats.stddev.sum() > 5){
        return false;
    }

    FloatPixel color_ratio = stats.average / stats.average.sum();
    double distance = euclidean_distance(
        color_ratio,
        FloatPixel(0.337067, 0.325728, 0.337204)
    );

    return distance < 0.1;
}


BattleDialogDetector::BattleDialogDetector(VideoFeed& feed)
    : m_bottom(feed, 0.50, 0.89, 0.40, 0.07)
    , m_left(feed, 0.01, 0.82, 0.03, 0.07)
    , m_right(feed, 0.95, 0.87, 0.04, 0.09)
{}

bool BattleDialogDetector::detect(const QImage& screen) const{
    ImageStats bottom = pixel_stats(extract_box(screen, m_bottom));
    if (!is_dialog_grey(bottom)){
        return false;
    }
    ImageStats left = pixel_stats(extract_box(screen, m_left));
    if (!is_dialog_grey(left)){
        return false;
    }
    ImageStats right = pixel_stats(extract_box(screen, m_right));
    if (!is_dialog_grey(right)){
        return false;
    }

    if (euclidean_distance(left.average, right.average) > 10){
        return false;
    }
    if (left.average.sum() > bottom.average.sum()){
        return false;
    }
    if (right.average.sum() > bottom.average.sum()){
        return false;
    }

    return true;
}


}
}
}
