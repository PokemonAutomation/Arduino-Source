/*  Fishing Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "PokemonSwSh_MarkFinder.h"
#include "PokemonSwSh_FishingDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



FishingMissDetector::FishingMissDetector()
    : m_hook_box(0.1, 0.15, 0.8, 0.4)
    , m_miss_box(0.3, 0.9, 0.4, 0.05)
{}
void FishingMissDetector::make_overlays(OverlaySet& items) const{
    items.add(Qt::red, m_hook_box);
    items.add(Qt::red, m_miss_box);
}
bool FishingMissDetector::detect(const QImage& frame){
    QImage miss_image = extract_box(frame, m_miss_box);
    ImageStats miss_stats = image_stats(miss_image);
    if (!is_white(miss_stats)){
        return false;
    }

    QImage hook_image = extract_box(frame, m_hook_box);
    if (image_stddev(hook_image).sum() < 50){
        return false;
    }

    return true;
}
bool FishingMissDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}

FishingHookDetector::FishingHookDetector(VideoOverlay& overlay)
    : m_overlay(overlay)
    , m_hook_box(0.1, 0.15, 0.8, 0.4)
{}
void FishingHookDetector::make_overlays(OverlaySet& items) const{
    items.add(Qt::red, m_hook_box);
}
bool FishingHookDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    QImage hook_image = extract_box(frame, m_hook_box);

    std::vector<ImagePixelBox> exclamation_marks;
    find_marks(hook_image, &exclamation_marks, nullptr);
    for (const ImagePixelBox& mark : exclamation_marks){
        ImageFloatBox box = translate_to_parent(frame, m_hook_box, mark);
        box.x -= box.width * 1.5;
        box.width *= 4;
        box.height *= 1.5;
        m_marks.emplace_back(m_overlay, box, Qt::yellow);
    }

    return !exclamation_marks.empty();
}











}
}
}

