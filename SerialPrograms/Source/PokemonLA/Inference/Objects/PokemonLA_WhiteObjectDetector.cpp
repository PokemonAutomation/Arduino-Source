/*  White Object Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

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
    const QImage& image
){
    std::set<Color> threshold_set;
    for (const auto& item : detectors){
        const std::set<Color>& thresholds = item.first.thresholds();
        threshold_set.insert(thresholds.begin(), thresholds.end());
    }
    std::vector<Color> threshold_list(threshold_set.begin(), threshold_set.end());

    size_t filters = threshold_set.size();
    std::vector<PackedBinaryMatrix2> matrix(filters);
    {
        size_t c = 0;
        for (; c + 3 < filters; c += 4){
            compress4_rgb32_to_binary_range(
                image,
                matrix[c + 0], (uint32_t)threshold_list[c + 0], 0xffffffff,
                matrix[c + 1], (uint32_t)threshold_list[c + 1], 0xffffffff,
                matrix[c + 2], (uint32_t)threshold_list[c + 2], 0xffffffff,
                matrix[c + 3], (uint32_t)threshold_list[c + 3], 0xffffffff
            );
        }
        for (; c + 1 < filters; c += 2){
            compress2_rgb32_to_binary_range(
                image,
                matrix[c + 0], (uint32_t)threshold_list[c + 0], 0xffffffff,
                matrix[c + 1], (uint32_t)threshold_list[c + 1], 0xffffffff
            );
        }
        for (; c < filters; c++){
            matrix[c] = compress_rgb32_to_binary_range(
                image, (uint32_t)threshold_list[c + 0], 0xffffffff
            );
        }
    }

    for (size_t c = 0; c < filters; c++){
        auto finder = make_WaterfillIterator(matrix[c], 50);
//        WaterfillIterator finder(matrix[c], 50);
        WaterfillObject object;
        while (finder->find_next(object)){
            for (const auto& detector : detectors){
                const std::set<Color>& thresholds = detector.first.thresholds();
                if (thresholds.find(threshold_list[c]) != thresholds.end()){
                    detector.first.process_object(image, object);
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
//    cout << "boxes.size() = " << boxes.size() << endl;

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
//    cout << "m_detections.size() = " << m_detections.size() << endl;
}




WhiteObjectWatcher::WhiteObjectWatcher(
    VideoOverlay& overlay,
    const ImageFloatBox& box,
    std::vector<std::pair<WhiteObjectDetector&, bool>> detectors
)
    : VisualInferenceCallback("WhiteObjectWatcher")
    , m_box(box)
    , m_overlays(overlay)
    , m_detectors(detectors)
{}

void WhiteObjectWatcher::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool WhiteObjectWatcher::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    for (auto& detector : m_detectors){
        detector.first.clear();
    }

    find_overworld_white_objects(m_detectors, extract_box(frame, m_box));
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
