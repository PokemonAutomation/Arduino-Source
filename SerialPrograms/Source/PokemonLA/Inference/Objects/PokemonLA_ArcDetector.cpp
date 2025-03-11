/*  Arc Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/SubObjectTemplateMatcher.h"
#include "PokemonLA_ArcDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels;
using namespace Kernels::Waterfill;


class ArcMatcher : public ImageMatch::SubObjectTemplateMatcher{
public:
    ArcMatcher(bool left)
        : SubObjectTemplateMatcher(
            left
                ? "PokemonLA/ArcL-Template0.png"
                : "PokemonLA/ArcR-Template0.png",
            80
        )
    {
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
            m_matcher.image_template(),
            128, 255,
            128, 255,
            128, 255
        );
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20);
        if (objects.size() != 1){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Failed to find exactly one object in resource.",
                m_path
            );
        }
        set_subobject(objects[0]);
    }

    static const ArcMatcher& left(){
        static ArcMatcher matcher(true);
        return matcher;
    }
    static const ArcMatcher& right(){
        static ArcMatcher matcher(false);
        return matcher;
    }
};




ArcDetector::ArcDetector()
    : WhiteObjectDetector(
        COLOR_GREEN,
        {
            Color(0xff808080),
            Color(0xff909090),
            Color(0xffa0a0a0),
            Color(0xffb0b0b0),
        }
    )
{}
void ArcDetector::process_object(const ImageViewRGB32& image, const WaterfillObject& object){
    ImagePixelBox object_box;
    if (ArcMatcher::left().matches(object_box, image, object)){
        m_left.emplace_back(object_box);
    }
    if (ArcMatcher::right().matches(object_box, image, object)){
        m_right.emplace_back(object_box);
    }
}
void ArcDetector::finish(const ImageViewRGB32& image){
    //  Merge left/right arcs.
    for (auto iter0 = m_left.begin(); iter0 != m_left.end();){
        double height = (double)iter0->height();
        bool removed = false;
        for (auto iter1 = m_right.begin(); iter1 != m_right.end(); ++iter1){
            double height_ratio = height / iter1->height();
            if (height_ratio < 0.9 || height_ratio > 1.1){
                continue;
            }

            double vertical_offset = ((ptrdiff_t)iter0->min_y - (ptrdiff_t)iter1->min_y) / height;
            if (std::abs(vertical_offset) > 0.1){
                continue;
            }

            double horizontal_offset = ((ptrdiff_t)iter1->min_x - (ptrdiff_t)iter0->min_x) / height - 1.27;
            if (horizontal_offset < -0.1 || horizontal_offset > 0.8){
                continue;
            }

            m_detections.emplace_back(
                iter0->min_x,
                std::min(iter0->min_y, iter1->min_y),
                iter1->max_x,
                std::max(iter0->max_y, iter1->max_y)
            );
            iter0 = m_left.erase(iter0);
            m_right.erase(iter1);
            removed = true;
            break;
        }
        if (!removed){
            ++iter0;
        }
    }
    m_left.clear();
    m_right.clear();
    merge_heavily_overlapping();
}






}
}
}
