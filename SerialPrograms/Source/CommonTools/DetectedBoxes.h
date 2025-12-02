/*  Detected Boxes
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_DetectedBoxes_H
#define PokemonAutomation_CommonTools_DetectedBoxes_H

#include <string>
#include <vector>
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{

struct DetectedBox{
    std::string name;
    ImageFloatBox box;
};

// Merge heavily overlapping pixel boxes. Input boxes is modified in place.
// Order detection boxes by their areas, then starting at the smallest box, trying to deduplicate
// boxes close to the current box.
// The duplication criteria is:
//   candidate_box.area <= current_box_area * (1+tolerance), and
//   overlap_area(candidate_box, current_box) * (1+tolerance) > current_box_area.
void merge_overlapping_boxes(std::vector<ImagePixelBox>& boxes, double tolerance = 0.2);

// Merge heavily overlapping pixel boxes. Input boxes is modified in place.
// Order detection boxes by their areas, then starting at the smallest box, trying to deduplicate
// boxes close to the current box.
// The duplication criteria is:
//   candidate_box.area <= current_box_area * (1+tolerance), and
//   overlap_area(candidate_box, current_box) * (1+tolerance) > current_box_area.
void merge_overlapping_boxes(std::vector<DetectedBox>& boxes, double tolerance = 0.2);


}
#endif
