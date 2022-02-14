/*  Flag Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageMatch/SubObjectTemplateMatcher.h"
#include "PokemonLA_FlagDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels;
using namespace Kernels::Waterfill;


void FlagMatcher_make_template(){
    QImage image("Flag-Original.png");
    image = image.scaled(image.width() / 2, image.height() / 2);
    image = image.convertToFormat(QImage::Format::Format_ARGB32);
    uint32_t* ptr = (uint32_t*)image.bits();
    size_t words = image.bytesPerLine() / sizeof(uint32_t);
    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            uint32_t& pixel = ptr[r * words + c];
            uint32_t red = qRed(pixel);
            uint32_t green = qGreen(pixel);
            uint32_t blue = qBlue(pixel);
//            if (red < 128 && green < 128 && blue < 128){
//                pixel = 0x00000000;
//            }
            if (red < 128 || green < 128 || blue < 128){
                pixel = 0x00000000;
            }
        }
    }
    image.save("Flag-Template0.png");
}




class FlagMatcher : public ImageMatch::SubObjectTemplateMatcher{
public:
    FlagMatcher(bool left)
        : SubObjectTemplateMatcher("PokemonLA/Flag-Template.png", 100)
    {
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
            m_object,
            128, 255,
            128, 255,
            128, 255
        );
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20, false);
        if (objects.size() != 2){
            PA_THROW_StringException("Failed to find exactly 2 objects in resource.");
        }

        if (left == (objects[0].min_x < objects[1].min_x)){
            set_subobject(objects[0]);
        }else{
            set_subobject(objects[1]);
        }
    }

    static const FlagMatcher& left(){
        static FlagMatcher matcher(true);
        return matcher;
    }
    static const FlagMatcher& right(){
        static FlagMatcher matcher(false);
        return matcher;
    }
};




FlagDetector::FlagDetector()
    : WhiteObjectDetector(
        COLOR_CYAN,
        {
            Color(0xff808080),
            Color(0xff909090),
            Color(0xffa0a0a0),
            Color(0xffb0b0b0),
        }
    )
{}
void FlagDetector::process_object(const QImage& image, const WaterfillObject& object){
    if (object.area < 100){
        return;
    }
    if (object.height() > 0.04 * image.height()){
        return;
    }
    if (object.height() < 0.01 * image.height()){
        return;
    }
    if (object.width() > 0.03 * image.width()){
        return;
    }
    ImagePixelBox object_box;
    if (FlagMatcher::left().matches(object_box, image, object)){
        m_left.emplace_back(object_box);
    }
    if (FlagMatcher::right().matches(object_box, image, object)){
        m_right.emplace_back(object_box);
    }
}
void FlagDetector::finish(){
//    cout << "left  = " << m_left.size() << endl;
//    cout << "right = " << m_right.size() << endl;

    //  Merge left/right parts.
    for (auto iter0 = m_left.begin(); iter0 != m_left.end();){
        double height = (double)iter0->height();
        double width = (double)iter0->width();
        bool removed = false;
        for (auto iter1 = m_right.begin(); iter1 != m_right.end(); ++iter1){
            double height_ratio = height / iter1->height();
            if (height_ratio < 0.8 || height_ratio > 1.2){
                continue;
            }
            double width_ratio = width / iter1->width();
            if (width_ratio < 0.8 || width_ratio > 1.2){
                continue;
            }

            double horizontal_offset = std::abs((iter0->min_x - iter1->min_x) / width);
            if (horizontal_offset > 0.1){
                continue;
            }

            double vertical_offset = std::abs((iter0->min_y - iter1->min_y) / height);
            if (vertical_offset > 0.1){
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
