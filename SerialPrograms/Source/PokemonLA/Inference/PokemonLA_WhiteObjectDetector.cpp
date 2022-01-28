/*  White Object Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageMatch/SubObjectTemplateMatcher.h"
#include "PokemonLA_WhiteObjectDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels;
using namespace Kernels::Waterfill;



void find_overworld_white_objects(
    const std::vector<std::pair<WhiteObjectDetector&, bool>>& detectors,
    const QImage& screen
){
    std::set<Color> threshold_set;
    for (const auto& item : detectors){
        const std::set<Color>& thresholds = item.first.thresholds();
        threshold_set.insert(thresholds.begin(), thresholds.end());
    }
    std::vector<Color> threshold_list(threshold_set.begin(), threshold_set.end());

    size_t filters = threshold_set.size();
    std::vector<PackedBinaryMatrix> matrix(filters);
    {
        size_t c = 0;
        for (; c + 3 < filters; c += 4){
            compress4_rgb32_to_binary_range(
                screen,
                matrix[c + 0], (uint32_t)threshold_list[c + 0], 0xffffffff,
                matrix[c + 1], (uint32_t)threshold_list[c + 1], 0xffffffff,
                matrix[c + 2], (uint32_t)threshold_list[c + 2], 0xffffffff,
                matrix[c + 3], (uint32_t)threshold_list[c + 3], 0xffffffff
            );
        }
        for (; c + 1 < filters; c += 2){
            compress2_rgb32_to_binary_range(
                screen,
                matrix[c + 0], (uint32_t)threshold_list[c + 0], 0xffffffff,
                matrix[c + 1], (uint32_t)threshold_list[c + 1], 0xffffffff
            );
        }
        for (; c < filters; c++){
            matrix[c] = compress_rgb32_to_binary_range(
                screen, (uint32_t)threshold_list[c + 0], 0xffffffff
            );
        }
    }

#if 0
    const size_t FILTERS = 4;
    PackedBinaryMatrix matrix[FILTERS];
    compress4_rgb32_to_binary_range(
        screen,
        matrix[0], 0xff909090, 0xffffffff,
        matrix[1], 0xffa0a0a0, 0xffffffff,
        matrix[2], 0xffb0b0b0, 0xffffffff,
        matrix[3], 0xffc0c0c0, 0xffffffff
    );
#endif

    for (size_t c = 0; c < filters; c++){
        WaterFillIterator finder(matrix[c], 50);
        WaterfillObject object;
        while (finder.find_next(object)){
            for (const auto& detector : detectors){
                const std::set<Color>& thresholds = detector.first.thresholds();
                if (thresholds.find(threshold_list[c]) != thresholds.end()){
                    detector.first.process_object(screen, object);
                }
            }
        }
    }

    for (const auto& detector : detectors){
        detector.first.finish();
    }
}


void WhiteObjectDetector::merge_heavily_overlapping(double tolerance){
    std::multimap<size_t, ImagePixelBox> boxes;
    for (const ImagePixelBox& box : m_detections){
        boxes.emplace(box.area(), box);
    }
    m_detections.clear();

    double ratio = 1.0 + tolerance;

    while (!boxes.empty()){
        auto current = boxes.begin();
        size_t limit = (size_t)(current->first * ratio);
        auto candidate = current;
        ++candidate;
        while (candidate != boxes.end()){
            if (candidate->first > limit){
                ++candidate;
                continue;
            }
            size_t overlap = current->second.overlap_with(candidate->second);
            if ((double)overlap * ratio > current->first){
                current->second.merge_with(candidate->second);
                candidate = boxes.erase(candidate);
            }else{
                ++candidate;
            }
        }
        m_detections.emplace_back(current->second);
        current = boxes.erase(current);
    }
}




WhiteObjectWatcher::WhiteObjectWatcher(
    VideoOverlay& overlay,
    std::vector<std::pair<WhiteObjectDetector&, bool>> detectors
)
    : VisualInferenceCallback("WhiteObjectWatcher")
    , m_box(0, 0, 1, 1)
    , m_overlays(overlay)
    , m_detectors(detectors)
{}

void WhiteObjectWatcher::make_overlays(VideoOverlaySet& items) const{}
bool WhiteObjectWatcher::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    for (auto& detector : m_detectors){
        detector.first.clear();
    }

    find_overworld_white_objects(m_detectors, frame);
    m_overlays.clear();

    for (auto& detector : m_detectors){
        for (const ImagePixelBox& obj : detector.first.detections()){
            ImageFloatBox box = translate_to_parent(frame, m_box, obj);
            m_overlays.add(detector.first.color(), box);
        }
    }
    for (auto& detector : m_detectors){
        if (detector.second && !detector.first.detections().empty()){
            return true;
        }
    }
    return false;
}





}
}
}
