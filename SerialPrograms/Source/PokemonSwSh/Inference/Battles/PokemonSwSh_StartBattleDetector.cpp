/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "PokemonSwSh_StartBattleDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{





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

    //  White screen.
    if (stats.average.sum() > 600 && stats.stddev.sum() < 10){
        return true;
    }

    //  Grey text box.
    bool dialog = stats.stddev.sum() > 50;
    dialog &= m_dialog.detect(frame);
    if (dialog){
//        cout << stats0.stddev.sum() << endl;
    }
    return dialog;
}



#if 0
bool wait_for_start_battle(
    ProgramEnvironment& env,
    VideoFeed& feed, VideoOverlay& overlay,
    std::chrono::milliseconds timeout
){
    StartBattleDetector detector(overlay);
    VisualInferenceWait inference(env, feed, overlay, timeout);
    inference += detector;
    return inference.run();
}
#endif





}
}
}
