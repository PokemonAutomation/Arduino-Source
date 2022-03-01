/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "CommonFramework/InferenceInfra/VisualInferenceRoutines.h"
#include "PokemonBDSP_StartBattleDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{





StartBattleDetector::StartBattleDetector(VideoOverlay& overlay)
    : VisualInferenceCallback("StartBattleDetector")
    , m_screen_box(0.2, 0.2, 0.6, 0.6)
{}
void StartBattleDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_screen_box);
    m_dialog.make_overlays(items);
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


StartBattleMenuOverlapDetector::StartBattleMenuOverlapDetector(VideoOverlay& overlay)
    : VisualInferenceCallback("StartBattleMenuOverlapDetector")
    , m_left(0.02, 0.2, 0.08, 0.5)
    , m_right(0.90, 0.2, 0.08, 0.5)
    , m_battle_detected(false)
{}
void StartBattleMenuOverlapDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_left);
    items.add(COLOR_RED, m_right);
}
bool StartBattleMenuOverlapDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    if (detect(frame)){
        m_battle_detected.store(true, std::memory_order_release);
        return true;
    }
    return false;
}
bool StartBattleMenuOverlapDetector::detect(const QImage& frame){
    QImage image0 = extract_box(frame, m_left);
    ImageStats stats0 = image_stats(image0);
    //  Solid screen that's not black.
    if (stats0.average.sum() < 50 || stats0.stddev.sum() > 10){
        return false;
    }

    QImage image1 = extract_box(frame, m_right);
    ImageStats stats1 = image_stats(image1);
    //  Solid screen that's not black.
    if (stats1.average.sum() < 50 && stats1.stddev.sum() > 10){
        return false;
    }

    return true;
}




}
}
}
