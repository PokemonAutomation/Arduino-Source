/*  Max Lair Detect Professor Swap
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Compiler.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonSwSh_MaxLair_Detect_ProfessorSwap.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


ProfessorSwapDetector::ProfessorSwapDetector(VideoOverlay& overlay)
    : SelectionArrowFinder(overlay, ImageFloatBox(0.600, 0.600, 0.200, 0.200))
//    , m_dialog0(0.257, 0.807, 0.015, 0.030)
//    , m_dialog1(0.710, 0.880, 0.030, 0.050)
    , m_bottom_main(0.100, 0.970, 0.600, 0.020)
{
}
void ProfessorSwapDetector::make_overlays(VideoOverlaySet& items) const{
    SelectionArrowFinder::make_overlays(items);
//    items.add(COLOR_YELLOW, m_dialog0);
//    items.add(COLOR_YELLOW, m_dialog1);
    items.add(COLOR_YELLOW, m_bottom_main);
}
bool ProfessorSwapDetector::detect(const QImage& screen){
//    ImageStats dialog_left = image_stats(extract_box_shallow(screen, m_dialog0));
//    if (!is_grey(dialog_left, 0, 200)){
//        return false;
//    }
//    ImageStats dialog_right = image_stats(extract_box_shallow(screen, m_dialog1));
//    if (!is_grey(dialog_right, 0, 200)){
//        return false;
//    }
    ImageStats dialog_left = image_stats(extract_box_shallow(screen, m_bottom_main));
    if (!is_black(dialog_left)){
        return false;
    }
    return SelectionArrowFinder::detect(screen);
}
bool ProfessorSwapDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}




}
}
}
}
