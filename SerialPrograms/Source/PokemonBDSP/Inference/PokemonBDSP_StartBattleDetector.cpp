/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "PokemonBDSP_StartBattleDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{





StartBattleDetector::StartBattleDetector(VideoOverlay& overlay)
    : m_screen_box(0.2, 0.2, 0.6, 0.6)
    , m_dialog(overlay)
{
    add_box(m_screen_box);
}
bool StartBattleDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}

bool StartBattleDetector::detect(const QImage& frame){
    QImage image = extract_box(frame, m_screen_box);

    ImageStats stats = image_stats(image);

    //  Solid screen that's not black.
    if (stats.average.sum() > 50 && stats.stddev.sum() < 10){
        return true;
    }

    return m_dialog.detect(frame);
}






}
}
}
