/*  Bubble Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/SubObjectTemplateMatcher.h"
#include "PokemonLA_BubbleDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels;
using namespace Kernels::Waterfill;


class BubbleMatcher : public ImageMatch::SubObjectTemplateMatcher{
public:
    BubbleMatcher()
        : SubObjectTemplateMatcher("PokemonLA/Bubble-Template0.png", 40)
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

    virtual bool check_image(const ImageViewRGB32& image) const override{
        return image_stddev(image).sum() > 100;
    };

    static const BubbleMatcher& instance(){
        static BubbleMatcher matcher;
        return matcher;
    }
};



BubbleDetector::BubbleDetector()
    : WhiteObjectDetector(COLOR_GREEN, {Color(0xffb0b0b0)})
{}
void BubbleDetector::process_object(const ImageViewRGB32& image, const WaterfillObject& object){
    if (object.area < 200){
        return;
    }
    if (object.width() < 0.03 * image.width()){
        return;
    }
    if (object.width() > 0.06 * image.width()){
        return;
    }
    ImagePixelBox object_box;
    if (BubbleMatcher::instance().matches(object_box, image, object)){
        m_detections.emplace_back(object_box);
    }
}
void BubbleDetector::finish(const ImageViewRGB32& image){
    merge_heavily_overlapping();
}



}
}
}
