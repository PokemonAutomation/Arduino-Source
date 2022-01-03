/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "PokemonSwSh_MarkFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Kernels;
using namespace Kernels::Waterfill;



const QImage& EXCLAMATION_TOP(){
    static QImage image(RESOURCE_PATH() + "PokemonSwSh/ExclamationTop.png");
    return image;
}
const QImage& QUESTION_TOP(){
    static QImage image(RESOURCE_PATH() + "PokemonSwSh/QuestionTop.png");
    return image;
}

bool is_exclamation_mark(const QImage& image, const WaterFillObject& object){
    size_t width = object.width();
    size_t height = object.height();

    double aspect_ratio = object.aspect_ratio();
    if (!(0.3 < aspect_ratio && aspect_ratio < 0.7)){
        return false;
    }

    const QImage& exclamation_mark = EXCLAMATION_TOP();
    QImage scaled = image.copy(
        (pxint_t)object.min_x, (pxint_t)object.min_y,
        (pxint_t)width, (pxint_t)height
    );
    scaled = scaled.scaled(exclamation_mark.width(), exclamation_mark.height());
    double rmsd = ImageMatch::pixel_RMSD(exclamation_mark, scaled);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd <= 80;
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

    const QImage& exclamation_mark = QUESTION_TOP();
    QImage scaled = image.copy(
        (pxint_t)object.min_x, (pxint_t)object.min_y,
        (pxint_t)width, (pxint_t)height
    );
    scaled = scaled.scaled(exclamation_mark.width(), exclamation_mark.height());
    double rmsd = ImageMatch::pixel_RMSD(exclamation_mark, scaled);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd <= 80;
}

std::vector<ImagePixelBox> find_exclamation_marks(const QImage& image){
    PackedBinaryMatrix matrix = filter_rgb32_range(
        image,
        192, 255,
        0, 160,
        0, 192
    );
    std::vector<WaterFillObject> objects = find_objects_inplace(matrix, 50, false);
    std::vector<ImagePixelBox> ret;
    for (const WaterFillObject& object : objects){
        if (is_exclamation_mark(image, object)){
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
std::vector<ImagePixelBox> find_question_marks(const QImage& image){
    PackedBinaryMatrix matrix = filter_rgb32_range(
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
