/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_BoxGenderDetector_H
#define PokemonAutomation_Pokemon_BoxGenderDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class VideoOverlaySet;

namespace Pokemon{

// Detect gender symbol inside the pokemon storage box
class BoxGenderDetector{
public:
    // box: the area where the gender symbol will appear
    // area_ratio_threshold: if the number of red pixel count > box total area * area_ratio_threshold, female symbol is detected
    //                       if the number of blue pixel count > box total area * area_ratio_threshold, male symbol is detected
    BoxGenderDetector(const ImageFloatBox& box, double area_ratio_threshold, Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;
    StatsHuntGenderFilter detect(const ImageViewRGB32& screen) const;

private:
    ImageFloatBox m_box;
    double m_area_ratio_threshold;
    Color m_color;
};



}
}

#endif

