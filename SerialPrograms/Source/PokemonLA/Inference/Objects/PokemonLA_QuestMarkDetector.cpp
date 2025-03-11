/*  Quest Mark Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/SubObjectTemplateMatcher.h"
#include "PokemonLA_QuestMarkDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels;
using namespace Kernels::Waterfill;



class QuestMarkMatcher : public ImageMatch::SubObjectTemplateMatcher{
public:
    QuestMarkMatcher()
        : SubObjectTemplateMatcher("PokemonLA/QuestMark-Template1.png", 100)
    {
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
            m_matcher.image_template(),
            128, 255,
            128, 255,
            128, 255
        );
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 100);

        //  Get largest white object.
        size_t area = 0;
        size_t index = 0;
        for (size_t c = 0; c < objects.size(); c++){
            if (area < objects[c].area){
                area = objects[c].area;
                index = c;
            }
        }

        set_subobject(objects[index]);
    }

    static const QuestMarkMatcher& instance(){
        static QuestMarkMatcher matcher;
        return matcher;
    }
};



QuestMarkDetector::QuestMarkDetector()
    : WhiteObjectDetector(
        COLOR_CYAN,
        {
//            Color(0xff808080),
            Color(0xff909090),
//            Color(0xffa0a0a0),
//            Color(0xffb0b0b0),
        }
    )
{}
void QuestMarkDetector::process_object(const ImageViewRGB32& image, const WaterfillObject& object){
    ImagePixelBox object_box;
    if (QuestMarkMatcher::instance().matches(object_box, image, object)){
        m_detections.emplace_back(object_box);
    }
    merge_heavily_overlapping();
}





}
}
}
