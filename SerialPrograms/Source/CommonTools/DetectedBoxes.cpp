/*  Detected Boxes
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "DetectedBoxes.h"

namespace PokemonAutomation{


void merge_overlapping_boxes(
    std::vector<ImagePixelBox>& input_boxes,
    double tolerance
){
    std::multimap<size_t, ImagePixelBox> boxes; // detection box area -> box
    for (const ImagePixelBox& box : input_boxes){
        boxes.emplace(box.area(), box);
    }
    input_boxes.clear();
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
        input_boxes.emplace_back(current->second);
        boxes.erase(current);
    }
}


void merge_overlapping_boxes(
    std::vector<DetectedBox>& input_boxes,
    double tolerance
){
    std::multimap<double, DetectedBox> boxes; // detection box area -> box
    for (const DetectedBox& box : input_boxes){
        boxes.emplace(box.box.area(), box);
    }
    input_boxes.clear();
//    cout << "boxes.size() = " << boxes.size() << endl;

    // default ratio is 1.2
    double ratio = 1.0 + tolerance;

    while (!boxes.empty()){
        // Start from the smallest box:
        auto current = boxes.begin();
        const double current_area = current->first;
        // get a max limit of area, e.g. current_box_area * 1.2
        const double limit = (current_area * ratio);
        auto candidate = current;
        ++candidate;
        while (candidate != boxes.end()){
            // skip if candidate area > limit
            if (candidate->first > limit){
                break;
            }
            // candiate area <= limit, further testing there overlapped area
            double overlap_area = current->second.box.overlapping_area(candidate->second.box);
            if (overlap_area * ratio > current_area){
                current->second.box.merge_with(candidate->second.box);
                candidate = boxes.erase(candidate);
            }else{
                ++candidate;
            }
        }
        input_boxes.emplace_back(current->second);
        boxes.erase(current);
    }
}


}
