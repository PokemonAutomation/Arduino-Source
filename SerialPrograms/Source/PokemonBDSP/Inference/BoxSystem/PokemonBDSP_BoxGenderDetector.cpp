/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/SubObjectTemplateMatcher.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
#include "PokemonBDSP_BoxGenderDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Kernels;
using namespace Kernels::Waterfill;
using namespace Pokemon;



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
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
            m_matcher.image_template(), 0xff7f7f7f, 0xffffffff
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




bool is_male(const ImageViewRGB32& image, const WaterfillObject& object){
    size_t width = object.width();
    size_t height = object.height();
    if (width > 2 * height){
        return false;
    }
    if (height > 2 * width){
        return false;
    }

    ImagePixelBox obj;
    return GenderIcon::male().matches(obj, image, object);
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

    ImagePixelBox obj;
    return GenderIcon::female().matches(obj, image, object);
}


StatsHuntGenderFilter read_gender_from_box(Logger& logger, VideoOverlay& overlay, const ImageViewRGB32& frame)
{
    OverlayBoxScope gender_box(overlay, {0.733, 0.022, 0.204, 0.049}, COLOR_BLUE);
    ImageViewRGB32 name_and_gender = extract_box_reference(frame, gender_box);

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(name_and_gender, 0xff7f7f7f, 0xffffffff);
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20);
    for (WaterfillObject& object : objects){
        if (is_male(name_and_gender, object)){
            logger.log("Detected male symbol.", COLOR_PURPLE);
            return StatsHuntGenderFilter::Male;
        }
        if (is_female(name_and_gender, object)){
            logger.log("Detected female symbol.", COLOR_PURPLE);
            return StatsHuntGenderFilter::Female;
        }
    }

    logger.log("No gender symbol detected.", COLOR_PURPLE);
    return StatsHuntGenderFilter::Genderless;
}




}
}
}
