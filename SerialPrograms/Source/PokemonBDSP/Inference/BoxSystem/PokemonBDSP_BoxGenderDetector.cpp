#include "PokemonBDSP_BoxGenderDetector.h"
#include <QImage>
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "PokemonBDSP/Options/PokemonBDSP_EggHatchFilter.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "CommonFramework/Globals.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"

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

bool is_male(const ConstImageRef& image, const WaterfillObject& object,LoggerQt& logger){
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

bool is_female(const ConstImageRef& image, const WaterfillObject& object,LoggerQt& logger){
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


EggHatchGenderFilter find_gender(const ConstImageRef& image, LoggerQt& logger){
    //PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(image, combine_rgb(0, 0, 130),combine_rgb(0, 75, 250));
    //PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(image, 0, 64, 0, 64, 128, 255);
    PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(image, 0, 130, 0, 130, 0, 255);
//    logger.log(matrix.dump());
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 200);
    for (const WaterfillObject& object : objects){
        if (is_male(image, object, logger)){
            return EggHatchGenderFilter::Male;
        }
    }

    matrix = compress_rgb32_to_binary_range(image, 0, 255, 0, 130, 0, 130);
//logger.log(matrix.dump());
    objects = find_objects_inplace(matrix, 200);
    for (const WaterfillObject& object : objects){
        if (is_female(image, object,logger)){
            return EggHatchGenderFilter::Female;
        }
    }

    return EggHatchGenderFilter::Genderless;
}

EggHatchGenderFilter read_gender(LoggerQt& logger, VideoOverlay& overlay,const QImage& frame, QString name)
{
    InferenceBoxScope gender_box(overlay, 0.733, 0.022, 0.204, 0.049, COLOR_BLUE);
    QImage name_and_gender = extract_box_reference(frame, gender_box).to_qimage();

    name_and_gender.save("D:\\Side-Projects\\PokemonAutomation\\PythonHelper\\images\\process\\" + name+ "_pre.png");

    EggHatchGenderFilter gender;

    gender = find_gender(name_and_gender, logger);

    if(gender == EggHatchGenderFilter::Male){
        logger.log("Male", COLOR_BLUE);
    }else if(gender == EggHatchGenderFilter::Female){
        logger.log("Female", COLOR_RED);
    }else if(gender == EggHatchGenderFilter::Genderless){
        logger.log("Genderless", COLOR_GRAY);
    }


//    InferenceBoxScope gender_box(overlay, 0.733, 0.022, 0.204, 0.049, COLOR_BLUE);
////0xffa0a0a0, 0xffffffff
//    QImage name_and_gender = extract_box_reference(frame, gender_box).to_qimage();
//    name_and_gender.save("D:\\Side-Projects\\PokemonAutomation\\PythonHelper\\images\\process\\" + name+ "_pre.png");
//    //filter_rgb32_range(name_and_gender, combine_rgb(96,148,189), 0xffffffff, Color(0), true);
//    filter_rgb32_range(name_and_gender, combine_rgb(91,159,212), 0xffffffff, Color(0), true);
//    name_and_gender.save("D:\\Side-Projects\\PokemonAutomation\\PythonHelper\\images\\process\\" + name + "_post.png");
//    const ImageStats gender_stats = image_stats(name_and_gender);

//    logger.log("R: " + std::to_string(gender_stats.average.r)+ " G: " + std::to_string(gender_stats.average.g) + " B: " + std::to_string(gender_stats.average.b));

////    if (is_solid(gender_stats, {0.333333, 0.333333, 0.333333}, 0.1, 10)){
////        logger.log("Gender: Genderless");
////    }else
//    if (gender_stats.average.b > gender_stats.average.g + 20 && gender_stats.average.b > gender_stats.average.r + 20){
//        logger.log("Gender: Male");
//        return EggHatchGenderFilter::Male;
//    }else if (gender_stats.average.r > gender_stats.average.g + 20 && gender_stats.average.r > gender_stats.average.b + 20){
//        logger.log("Gender: Female");
//        return EggHatchGenderFilter::Female;
//    }else{
//        logger.log("Gender: Unable to detect", COLOR_RED);
//    }

    return EggHatchGenderFilter::Any;
}




}
}
}
