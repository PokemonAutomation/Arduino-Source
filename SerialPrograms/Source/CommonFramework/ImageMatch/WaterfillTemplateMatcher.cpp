/*  Waterfill Template Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "WaterfillTemplateMatcher.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{

using namespace Kernels;
using namespace Kernels::Waterfill;


WaterfillTemplateMatcher::WaterfillTemplateMatcher(
    const char* path,
    Color min_color, Color max_color,
    size_t min_area
){
    std::string full_path = RESOURCE_PATH() + path;
    ImageRGB32 reference(full_path);

    PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(reference, (uint32_t)min_color, (uint32_t)max_color);
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, min_area);
    if (objects.empty()){
        throw FileException(
            nullptr, PA_CURRENT_FUNCTION,
            "Failed to find any waterfill objects in resource template file.",
            std::move(full_path)
        );
    }

    const WaterfillObject* best = &objects[0];
    for (const WaterfillObject& object : objects){
        if (best->area < object.area){
            best = &object;
        }
    }

    m_matcher.reset(new ExactImageMatcher(extract_box_reference(reference, *best).copy()));
    m_area_ratio = best->area_ratio();
}

double WaterfillTemplateMatcher::rmsd(const ImageViewRGB32& image) const{
    if (!image || !check_image(image)){
        return 99999.;
    }
    return m_matcher->rmsd(image);
}
bool WaterfillTemplateMatcher::check_aspect_ratio(size_t candidate_width, size_t candidate_height) const{
    ImageViewRGB32 image_template = m_matcher->image_template();

    double error = (double)image_template.width() * candidate_height;
    error /= (double)image_template.height() * candidate_width;
    // cout << "ratio = " << error << endl;
    return m_aspect_ratio_lower <= error && error <= m_aspect_ratio_upper;
}
bool WaterfillTemplateMatcher::check_area_ratio(double candidate_area_ratio) const{
    if (m_area_ratio == 0){
        return true;
    }
    double error = candidate_area_ratio / m_area_ratio;
    // cout << "template area ratio " << m_area_ratio << " candidate " << candidate_area_ratio << " area error = " << error << endl;
    return m_area_ratio_lower <= error && error <= m_area_ratio_upper;
}
double WaterfillTemplateMatcher::rmsd_precropped(const ImageViewRGB32& cropped_image, const WaterfillObject& object) const{

    // XXX
    // dump_debug_image(global_logger_command_line(), "CommonFramework/WaterfillTemplateMatcher", "rmsd_precropped_input", cropped_image);

    if (!check_aspect_ratio(object.width(), object.height())){
        // cout << "bad aspect ratio" << endl;
        return 99999.;
    }
    if (!check_area_ratio(object.area_ratio())){
        // cout << "bad area ratio" << endl;
        return 99999.;
    }

//    static int c = 0;
//    cout << c << endl;

    double rmsd = this->rmsd(cropped_image);

//    cout << "rmsd  = " << rmsd << endl;

//    if (rmsd <= m_max_rmsd){
//        static int c = 0;
//        cropped_image.save("test-" + std::to_string(c++) + "-" + std::to_string(rmsd) + ".png");
//    }

    return rmsd;
}
double WaterfillTemplateMatcher::rmsd_original(const ImageViewRGB32& original_image, const WaterfillObject& object) const{

    // XXX
    // dump_debug_image(global_logger_command_line(), "CommonFramework/WaterfillTemplateMatcher", "rmsd_original_input", extract_box_reference(original_image, object));

    if (!check_aspect_ratio(object.width(), object.height())){
        // cout << "bad aspect ratio" << endl;
        // double expected_aspect_ratio = (double)m_object.width() / m_object.height();
        // double actual_aspect_ratio = (double)object.width() / object.height();
        // cout << "expected_aspect_ratio = " << expected_aspect_ratio << endl;
        // cout << "actual_aspect_ratio = " << actual_aspect_ratio << endl;
        return 99999.;
    }
    if (!check_area_ratio(object.area_ratio())){
        // cout << "bad area ratio" << endl;
        return 99999.;
    }

//    static int c = 0;
//    cout << c << endl;

    double rmsd = this->rmsd(extract_box_reference(original_image, object));

//    cout << "rmsd  = " << rmsd << endl;

//    if (rmsd <= m_max_rmsd){
//        static int c = 0;
//        extract_box(image, object).save("test-" + std::to_string(c++) + "-" + std::to_string(rmsd) + ".png");
//    }

    return rmsd;
}




}
}
