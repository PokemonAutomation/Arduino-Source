/*  Battle Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



BattleDialogDetector::BattleDialogDetector(VideoOverlay& overlay)
    : m_bottom(overlay, 0.50, 0.89, 0.40, 0.07)
    , m_left(overlay, 0.01, 0.82, 0.03, 0.07)
    , m_right(overlay, 0.97, 0.87, 0.02, 0.09)
{}

bool BattleDialogDetector::detect(const QImage& screen) const{
    ImageStats bottom = image_stats(extract_box(screen, m_bottom));
    if (!is_grey(bottom, 0, 200)){
        return false;
    }
    ImageStats left = image_stats(extract_box(screen, m_left));
    if (!is_grey(left, 0, 200)){
        return false;
    }
    ImageStats right = image_stats(extract_box(screen, m_right));
    if (!is_grey(right, 0, 200)){
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
