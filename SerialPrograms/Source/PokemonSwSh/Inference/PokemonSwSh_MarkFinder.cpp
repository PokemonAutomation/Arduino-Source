/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "PokemonSwSh_MarkFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Kernels;
using namespace Kernels::Waterfill;



class ExclamationMatcher{
public:
    ExclamationMatcher()
        : m_object(RESOURCE_PATH() + "PokemonSwSh/ExclamationMark.png")
        , m_matcher(m_object)
    {
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
            m_object,
            192, 255,
            0, 160,
            0, 192
        );
        std::vector<WaterFillObject> objects = find_objects_inplace(matrix, 50, false);
        if (objects.size() != 1){
            PA_THROW_StringException("Failed to find exactly one object in resource.");
        }
        m_feature_box = pixelbox_to_floatbox(
            m_object,
            ImagePixelBox(
                objects[0].min_x, objects[0].min_y,
                objects[0].max_x, objects[0].max_y
            )
        );
//        cout << m_feature_box.x << ", "
//             << m_feature_box.y << ", "
//             << m_feature_box.width << ", "
//             << m_feature_box.height << endl;
    }

    static const ExclamationMatcher& instance(){
        static ExclamationMatcher matcher;
        return matcher;
    }

    double rmsd(
        ImagePixelBox& object_box,
        const QImage& image,
        const ImagePixelBox& exclamation_top
    ) const{
        object_box = extract_object_from_inner_feature(exclamation_top, m_feature_box);
//        cout << object_box.min_x << ", "
//             << object_box.min_y << ", "
//             << object_box.max_x << ", "
//             << object_box.max_y << endl;
        QImage object = extract_box(image, object_box);
//        object.save("test.png");
        if (object.isNull()){
            return 99999.;
        }
        return m_matcher.rmsd(object);
    }


private:
    QImage m_object;
    ImageMatch::ExactImageMatcher m_matcher;
    ImageFloatBox m_feature_box;
};





const ImageMatch::ExactImageMatcher& QUESTION_TOP(){
    static ImageMatch::ExactImageMatcher matcher(QImage(RESOURCE_PATH() + "PokemonSwSh/QuestionTop.png"));
    return matcher;
}

bool is_exclamation_mark(ImagePixelBox& object_box, const QImage& image, const WaterFillObject& object){
    double aspect_ratio = object.aspect_ratio();
    if (!(0.3 < aspect_ratio && aspect_ratio < 0.7)){
        return false;
    }

    double rmsd = ExclamationMatcher::instance().rmsd(
        object_box, image,
        ImagePixelBox(object.min_x, object.min_y, object.max_x, object.max_y)
    );
//    if (rmsd <= 120){
//        cout << "is_exclamation_mark(): rmsd = " << rmsd << endl;
//    }
    return rmsd <= 120;
}
bool is_question_mark(const QImage& image, const WaterFillObject& object){
    size_t width = object.width();
    size_t height = object.height();

    if (width > 2 * height){
        return false;
    }
    if (height > 2 * width){
        return false;
    }

//    const QImage& exclamation_mark = QUESTION_TOP();
    QImage scaled = image.copy(
        (pxint_t)object.min_x, (pxint_t)object.min_y,
        (pxint_t)width, (pxint_t)height
    );
//    scaled = scaled.scaled(exclamation_mark.width(), exclamation_mark.height());
    double rmsd = QUESTION_TOP().rmsd(scaled);
//    double rmsd = ImageMatch::pixel_RMSD(exclamation_mark, scaled);
    if (rmsd <= 100){
//        cout << "is_question_mark(): rmsd = " << rmsd << endl;
    }
    return rmsd <= 100;
}

std::vector<ImagePixelBox> find_exclamation_marks(const QImage& image){
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        192, 255,
        0, 160,
        0, 192
    );
    std::vector<WaterFillObject> objects = find_objects_inplace(matrix, 50, false);
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
    for (const WaterFillObject& object : objects){
        ImagePixelBox object_box;
        if (is_exclamation_mark(object_box, image, object)){
            ret.emplace_back(object_box);
        }
    }
    return ret;
}
std::vector<ImagePixelBox> find_question_marks(const QImage& image){
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        0, 128,
        0, 255,
        128, 255
    );
    std::vector<WaterFillObject> objects = find_objects_inplace(matrix, 50, false);
    std::vector<ImagePixelBox> ret;
    for (const WaterFillObject& object : objects){
        if (is_question_mark(image, object)){
            ret.emplace_back(
                ImagePixelBox(
                    object.min_x, object.min_y,
                    object.max_x, object.max_y + object.height() / 3
                )
            );
        }
    }
    return ret;
}




}
}
}
