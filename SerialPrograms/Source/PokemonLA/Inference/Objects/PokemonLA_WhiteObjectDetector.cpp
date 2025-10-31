/*  White Object Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonLA_WhiteObjectDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels::Waterfill;



// hardcode white object found in waterfill to have min area threshold of 50
void find_overworld_white_objects(
    Resolution input_resolution,
    const std::vector<std::pair<WhiteObjectDetector&, bool>>& detectors,
    const ImageViewRGB32& image
){
    // collect color filters from all white object detectors
    std::set<Color> threshold_set;
    for (const auto& item : detectors){
        const std::set<Color>& thresholds = item.first.thresholds();
        threshold_set.insert(thresholds.begin(), thresholds.end());
    }

//    FixedLimitVector<CompressRgb32ToBinaryRangeFilter> filters(threshold_set.size());
//    for (Color filter : threshold_set){
//        filters.emplace_back(image.width(), image.height(), (uint32_t)filter, 0xffffffff);
//    }
//    compress_rgb32_to_binary_range(image, filters.data(), filters.size());

//    static int count = 0;
    {
        std::vector<std::pair<uint32_t, uint32_t>> filters;
        for (Color filter : threshold_set){
            filters.emplace_back((uint32_t)filter, 0xffffffff);
        }
        std::vector<PackedBinaryMatrix> matrix = compress_rgb32_to_binary_range(image, filters);

#if 1
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
        for (size_t c = 0; c < filters.size(); c++){

//            cout << "filter[" << c << "] = " << filters[c].first << endl;
//            cout << matrix[c].width() << " x " << matrix[c].height() << endl;
//            cout << matrix[c].dump() << endl;
            session->set_source(matrix[c]);
            auto finder = session->make_iterator(50);
            WaterfillObject object;
            while (finder->find_next(object, false)){
//                cout << object.area << endl;
//                cout << matrix[c].submatrix(object.min_x, object.min_y, object.width(), object.height()).dump() << endl;
//                extract_box_reference(image, object).save("test-" + std::to_string(count++) + "-" + std::to_string(c) + ".png");
                for (const auto& detector : detectors){
                    const std::set<Color>& thresholds = detector.first.thresholds();
                    if (thresholds.find((Color)filters[c].first) != thresholds.end()){
                        detector.first.process_object(input_resolution, image, object);
                    }
                }
            }
        }
#endif
    }

    for (const auto& detector : detectors){
        detector.first.finish(image);
    }
}


void WhiteObjectDetector::merge_heavily_overlapping(double tolerance){
    // Algorithm overview:
    // Order detection boxes by their areas, then starting at the smallest box, trying to deduplicate
    // boxes close to the current box.
    // the duplication criteria is:
    //   box.area <= current_box_area * (1+tolerance), and 
    //   overlap_area(box, current_box) * (1+tolerance) > current_box_area.

    std::multimap<size_t, ImagePixelBox> boxes; // detection box area -> box
    for (const ImagePixelBox& box : m_detections){
        boxes.emplace(box.area(), box);
    }
    m_detections.clear();
//    cout << "boxes.size() = " << boxes.size() << endl;

    // default ratio is 1.2
    double ratio = 1.0 + tolerance;

    while (!boxes.empty()){
        // Start from the smallest box:
        auto current = boxes.begin();
        const size_t current_area = current->first;
        // get a max limit of area, e.g. current_box_area * 1.2
        const size_t limit = (size_t)(current_area * ratio);
        auto candidate = current;
        ++candidate;
        while (candidate != boxes.end()){
            // skip if candidate area > limit
            if (candidate->first > limit){
                break;
            }
            // candiate area <= limit, further testing there overlapped area
            size_t overlap_area = current->second.overlapping_area(candidate->second);
            if ((double)overlap_area * ratio > current_area){
                current->second.merge_with(candidate->second);
                candidate = boxes.erase(candidate);
            }else{
                ++candidate;
            }
        }
        m_detections.emplace_back(current->second);
        boxes.erase(current);
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
bool WhiteObjectWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    for (auto& detector : m_detectors){
        detector.first.clear();
    }

    find_overworld_white_objects(
        frame.size(),
        m_detectors,
        extract_box_reference(frame, m_box)
    );
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
