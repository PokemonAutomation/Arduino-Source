/*  End Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "PokemonBDSP_EndBattleDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


EndBattleDetector::EndBattleDetector(const ImageFloatBox& box)
    : m_box(box)
    , m_has_been_black(false)
{
    add_box(m_box);
}


bool EndBattleDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return battle_is_over(frame);
}
bool EndBattleDetector::battle_is_over(const QImage& frame){
    QImage image = extract_box(frame, m_box);
    ImageStats stats = image_stats(image);
    if (is_black(stats)){
        m_has_been_black = true;
        return false;
    }
    if (!m_has_been_black){
        return false;
    }
    if (stats.stddev.sum() < 60){
        return false;
    }
    return true;
}



}
}
}
