/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "CommonFramework/ImageMatch/SubObjectTemplateMatcher.h"
#include "PokemonBDSP/Options/PokemonBDSP_EggHatchFilter.h"
#include "PokemonBDSP_BoxGenderDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Kernels;
using namespace Kernels::Waterfill;



class GenderIcon : public ImageMatch::SubObjectTemplateMatcher{
public:
    GenderIcon(bool male)
        : SubObjectTemplateMatcher(
            male
                ? "PokemonBDSP/M-Icon.png"
                : "PokemonBDSP/F-Icon.png",
            100
        )
    {
        PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(
            m_object, 0xff7f7f7f, 0xffffffff
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

    static const GenderIcon& female(){
        static GenderIcon matcher(false);
        return matcher;
    }
    static const GenderIcon& male(){
        static GenderIcon matcher(true);
        return matcher;
    }
};




#if 0
const ImageMatch::ExactImageMatcher& FEMALE_ICON(){
    static ImageMatch::ExactImageMatcher matcher(QImage(RESOURCE_PATH() + "PokemonBDSP/F-Icon.png"));
    return matcher;
}

const ImageMatch::ExactImageMatcher& MALE_ICON(){
    static ImageMatch::ExactImageMatcher matcher(QImage(RESOURCE_PATH() + "PokemonBDSP/M-Icon.png"));
    return matcher;
}
#endif

bool is_male(const ImageViewRGB32& image, const WaterfillObject& object){
    size_t width = object.width();
    size_t height = object.height();
    if (width > 2 * height){
        return false;
    }
    if (height > 2 * width){
        return false;
    }

#if 1
    ImagePixelBox obj;
    return GenderIcon::male().matches(obj, image, object);
#else
    ImageViewRGB32 obj = extract_box_reference(image, object);
    static int c = 0;
    obj.save("blue-" + QString::number(c++) + ".png");
//    image.save("image-" + QString::number(c++) + ".png");

    double rmsd = MALE_ICON().rmsd(obj);
    //logger.log("MALE RMSD:" + std::to_string(rmsd));
    cout << "male rmsd = " << rmsd << endl;
    return rmsd <= 110;
#endif
}

bool is_female(const ImageViewRGB32& image, const WaterfillObject& object){
    size_t width = object.width();
    size_t height = object.height();
    if (width > 2 * height){
        return false;
    }
    if (height > 2 * width){
        return false;
    }

#if 1
    ImagePixelBox obj;
    return GenderIcon::female().matches(obj, image, object);
#else
    ImageViewRGB32 obj = extract_box_reference(image, object);
    static int c = 0;
    obj.save("red-" + QString::number(c++) + ".png");
//    image.save("image-" + QString::number(c++) + ".png");

    double rmsd = FEMALE_ICON().rmsd(obj);
    //logger.log("FEMALE RMSD:" + std::to_string(rmsd));
    cout << "female rmsd = " << rmsd << endl;
    return rmsd <= 140;
#endif
}


EggHatchGenderFilter read_gender_from_box(LoggerQt& logger, VideoOverlay& overlay,const QImage& frame)
{
    InferenceBoxScope gender_box(overlay, 0.733, 0.022, 0.204, 0.049, COLOR_BLUE);
    ImageViewRGB32 name_and_gender = extract_box_reference(frame, gender_box);

    PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(name_and_gender, 0xff7f7f7f, 0xffffffff);
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20);
    for (WaterfillObject& object : objects){
        if (is_male(name_and_gender, object)){
            logger.log("Detected male symbol.", COLOR_PURPLE);
            return EggHatchGenderFilter::Male;
        }
        if (is_female(name_and_gender, object)){
            logger.log("Detected female symbol.", COLOR_PURPLE);
            return EggHatchGenderFilter::Female;
        }
    }

#if 0
    PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(name_and_gender, 0xff000040, 0xff3f3fff);
//    logger.log(matrix.dump());
//    cout << matrix.dump() << endl;

    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 50);
    for (const WaterfillObject& object : objects){
        if (is_male(name_and_gender, object)){
            logger.log("Male", COLOR_BLUE);
            return EggHatchGenderFilter::Male;
        }
    }

    matrix = compress_rgb32_to_binary_range(name_and_gender, 0xff800000, 0xffff3f3f);
//    logger.log(matrix.dump());
//    cout << matrix.dump() << endl;

    objects = find_objects_inplace(matrix, 50);
    for (const WaterfillObject& object : objects){
        if (is_female(name_and_gender, object)){
            logger.log("Female", COLOR_RED);
            return EggHatchGenderFilter::Female;
        }
    }
#endif

    logger.log("No gender symbol detected.", COLOR_PURPLE);
    return EggHatchGenderFilter::Genderless;
}




}
}
}
