/*  Battle Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonSwSh_BattleDialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



BattleDialogDetector::BattleDialogDetector(Color color)
    : m_color(color)
    , m_bottom(0.50, 0.89, 0.40, 0.07)
    , m_left(0.01, 0.82, 0.03, 0.07)
    , m_right(0.97, 0.87, 0.02, 0.09)
{}
void BattleDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom);
    items.add(m_color, m_left);
    items.add(m_color, m_right);
}
bool BattleDialogDetector::detect(const QImage& screen) const{
    ImageStats bottom = image_stats(extract_box(screen, m_bottom));
    if (!is_grey(bottom, 0, 200, 5)){
        return false;
    }
    ImageStats left = image_stats(extract_box(screen, m_left));
    if (!is_grey(left, 0, 200, 5)){
        return false;
    }
    ImageStats right = image_stats(extract_box(screen, m_right));
    if (!is_grey(right, 0, 200, 5)){
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
