/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "CommonTools/ImageMatch/SubObjectTemplateMatcher.h"
#include "PokemonSwSh_MarkFinder.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Kernels;
using namespace Kernels::Waterfill;


void generate_exclamation_mark(){
    ImageRGB32 image("ExclamationMark1.png");
    image = image.scale_to(image.width() / 4, image.height() / 4);
    uint32_t* ptr = image.data();
    size_t words = image.bytes_per_row() / sizeof(uint32_t);
    for (size_t r = 0; r < image.height(); r++){
        for (size_t c = 0; c < image.width(); c++){
            uint32_t& pixel = ptr[r * words + c];
            Color color(pixel);
            uint32_t red = color.red();
            uint32_t green = color.green();
            uint32_t blue = color.blue();
            if (red < 192 && green < 192){
                pixel = 0xff000000;
            }
            if (blue > red + 20){
                pixel = 0xff000000;
            }
        }
    }
    image.save("test.png");
}



class ExclamationMatcher : public ImageMatch::SubObjectTemplateMatcher{
public:
    static const ExclamationMatcher& instance(){
        static ExclamationMatcher matcher;
        return matcher;
    }

    ExclamationMatcher()
        : SubObjectTemplateMatcher("PokemonSwSh/ExclamationMark1-Template.png", 80)
    {
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
            m_matcher.image_template(),
            160, 255,
            0, 160,
            0, 192
        );
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20);
        if (objects.size() != 2){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Failed to find exactly two objects in resource.",
                m_path
            );
        }
        size_t index = 0;
        if (objects[0].area < objects[1].area){
            index = 1;
        }
        set_subobject(objects[index]);
    }

};





const ImageMatch::ExactImageMatcher& QUESTION_TOP(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSwSh/QuestionTop.png");
    return matcher;
}

bool is_question_mark(const ImageViewRGB32& image, const WaterfillObject& object){
    size_t width = object.width();
    size_t height = object.height();

    if (width > 2 * height){
        return false;
    }
    if (height > 2 * width){
        return false;
    }

    ImageViewRGB32 scaled = extract_box_reference(image, object);
//    scaled = scaled.scaled(exclamation_mark.width(), exclamation_mark.height());
    double rmsd = QUESTION_TOP().rmsd(scaled);
//    double rmsd = ImageMatch::pixel_RMSD(exclamation_mark, scaled);
    if (rmsd <= 100){
//        cout << "is_question_mark(): rmsd = " << rmsd << endl;
    }
    return rmsd <= 100;
}

std::vector<ImagePixelBox> find_exclamation_marks(const ImageViewRGB32& image){
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        160, 255,
        0, 160,
        0, 192
    );
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20);
#if 0
    cout << "objects = " << objects.size() << endl;
    static int c = 0;
    for (const auto& object : objects){
        extract_box_reference(image, object).save("test-" + std::to_string(c++) + ".png");
    }
#endif
    std::vector<ImagePixelBox> ret;
    for (const WaterfillObject& object : objects){
        if (object.area < 100){
            continue;
        }
        ImagePixelBox object_box;
        if (ExclamationMatcher::instance().matches(object_box, image, object)){
            ret.emplace_back(object_box);
//            static int c = 0;
//            extract_box_reference(image, object).save("test-" + std::to_string(c++) + ".png");
        }
    }
//    cout << ret.size() << endl;
    return ret;
}
std::vector<ImagePixelBox> find_question_marks(const ImageViewRGB32& image){
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        0, 128,
        0, 255,
        128, 255
    );
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 50);
    std::vector<ImagePixelBox> ret;
#if 1
    for (const WaterfillObject& object : objects){
        if (is_question_mark(image, object)){
            ret.emplace_back(
                ImagePixelBox(
                    object.min_x, object.min_y,
                    object.max_x, object.max_y + object.height() / 3
                )
            );
        }
    }
#endif
    return ret;
}




}
}
}
