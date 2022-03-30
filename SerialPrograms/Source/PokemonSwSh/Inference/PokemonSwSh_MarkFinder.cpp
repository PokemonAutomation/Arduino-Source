/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "CommonFramework/ImageMatch/SubObjectTemplateMatcher.h"
#include "PokemonSwSh_MarkFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Kernels;
using namespace Kernels::Waterfill;


void generate_exclamation_mark(){
    QImage image("ExclamationMark1.png");
    image = image.scaled(image.width() / 4, image.height() / 4);
    image = image.convertToFormat(QImage::Format::Format_ARGB32);
    uint32_t* ptr = (uint32_t*)image.bits();
    size_t words = image.bytesPerLine() / sizeof(uint32_t);
    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            uint32_t& pixel = ptr[r * words + c];
            uint32_t red = qRed(pixel);
            uint32_t green = qGreen(pixel);
            uint32_t blue = qBlue(pixel);
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
        PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(
            m_object,
            160, 255,
            0, 160,
            0, 192
        );
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20);
        if (objects.size() != 2){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Failed to find exactly two objects in resource.",
                m_path.toStdString()
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
    static ImageMatch::ExactImageMatcher matcher(QImage(RESOURCE_PATH() + "PokemonSwSh/QuestionTop.png"));
    return matcher;
}

bool is_question_mark(const ConstImageRef& image, const WaterfillObject& object){
    size_t width = object.width();
    size_t height = object.height();

    if (width > 2 * height){
        return false;
    }
    if (height > 2 * width){
        return false;
    }

//    const QImage& exclamation_mark = QUESTION_TOP();
    ConstImageRef scaled = extract_box_reference(image, object);
//    scaled = scaled.scaled(exclamation_mark.width(), exclamation_mark.height());
    double rmsd = QUESTION_TOP().rmsd(scaled);
//    double rmsd = ImageMatch::pixel_RMSD(exclamation_mark, scaled);
    if (rmsd <= 100){
//        cout << "is_question_mark(): rmsd = " << rmsd << endl;
    }
    return rmsd <= 100;
}

std::vector<ImagePixelBox> find_exclamation_marks(const ConstImageRef& image){
    PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(
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
        image.copy(
            object.min_x, object.min_y, object.width(), object.height()
        ).save("test-" + QString::number(c++) + ".png");
    }
#endif
    std::vector<ImagePixelBox> ret;
    for (const WaterfillObject& object : objects){
        ImagePixelBox object_box;
        if (ExclamationMatcher::instance().matches(object_box, image, object)){
            ret.emplace_back(object_box);
        }
    }
    return ret;
}
std::vector<ImagePixelBox> find_question_marks(const ConstImageRef& image){
    PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(
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
