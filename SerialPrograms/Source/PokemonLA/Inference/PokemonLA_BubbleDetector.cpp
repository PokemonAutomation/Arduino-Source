/*  Bubble Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageMatch/SubObjectTemplateMatcher.h"
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
            m_object,
            128, 255,
            128, 255,
            128, 255
        );
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20, false);
        if (objects.size() != 1){
            PA_THROW_StringException("Failed to find exactly one object in resource.");
        }
        set_subobject(objects[0]);
    }

    virtual bool check_image(const QImage& image) const{
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
void BubbleDetector::process_object(const QImage& screen, const WaterfillObject& object){
    if (object.area < 200){
        return;
    }
    if (object.width() < 0.03 * screen.width()){
        return;
    }
    if (object.width() > 0.06 * screen.width()){
        return;
    }
    ImagePixelBox object_box;
    if (BubbleMatcher::instance().matches(object_box, screen, object)){
        m_detections.emplace_back(object_box);
    }
}
void BubbleDetector::finish(){
    merge_heavily_overlapping();
}



}
}
}
