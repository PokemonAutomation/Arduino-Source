/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "PokemonBDSP/Options/PokemonBDSP_EggHatchFilter.h"
#include "PokemonBDSP_BoxGenderDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Kernels;
using namespace Kernels::Waterfill;


const ImageMatch::ExactImageMatcher& FEMALE_ICON(){
    static ImageMatch::ExactImageMatcher matcher(QImage(RESOURCE_PATH() + "PokemonBDSP/F-Icon.png"));
    return matcher;
}

const ImageMatch::ExactImageMatcher& MALE_ICON(){
    static ImageMatch::ExactImageMatcher matcher(QImage(RESOURCE_PATH() + "PokemonBDSP/M-Icon.png"));
    return matcher;
}

bool is_male(const ConstImageRef& image, const WaterfillObject& object){
    size_t width = object.width();
    size_t height = object.height();
    if (width > 2 * height){
        return false;
    }
    if (height > 2 * width){
        return false;
    }

    ConstImageRef obj = extract_box_reference(image, object);
//    static int c = 0;
//    obj.save("obj-" + QString::number(c++) + ".png");
//    image.save("image-" + QString::number(c++) + ".png");

    double rmsd = MALE_ICON().rmsd(obj);
    //logger.log("MALE RMSD:" + std::to_string(rmsd));
    return rmsd <= 110;
}

bool is_female(const ConstImageRef& image, const WaterfillObject& object){
    size_t width = object.width();
    size_t height = object.height();
    if (width > 2 * height){
        return false;
    }
    if (height > 2 * width){
        return false;
    }

    ConstImageRef obj = extract_box_reference(image, object);
    double rmsd = FEMALE_ICON().rmsd(obj);
    //logger.log("FEMALE RMSD:" + std::to_string(rmsd));
    return rmsd <= 140;
}


EggHatchGenderFilter read_gender_from_box(LoggerQt& logger, VideoOverlay& overlay,const QImage& frame)
{
    InferenceBoxScope gender_box(overlay, 0.733, 0.022, 0.204, 0.049, COLOR_BLUE);

    ConstImageRef name_and_gender = extract_box_reference(frame, gender_box);

    PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(name_and_gender, 0, 130, 0, 130, 0, 255);
    //logger.log(matrix.dump());
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 50);
    for (const WaterfillObject& object : objects){
        if (is_male(name_and_gender, object)){
            logger.log("Male", COLOR_BLUE);
            return EggHatchGenderFilter::Male;
        }
    }

    matrix = compress_rgb32_to_binary_range(name_and_gender, 0, 255, 0, 130, 0, 130);
    //logger.log(matrix.dump());
    objects = find_objects_inplace(matrix, 50);
    for (const WaterfillObject& object : objects){
        if (is_female(name_and_gender, object)){
            logger.log("Female", COLOR_RED);
            return EggHatchGenderFilter::Female;
        }
    }

    logger.log("Genderless", COLOR_GRAY);
    return EggHatchGenderFilter::Genderless;
}




}
}
}
