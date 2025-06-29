/*  Sub-Object Template Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/Globals.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "SubObjectTemplateMatcher.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{


SubObjectTemplateMatcher::SubObjectTemplateMatcher(const char* path, double max_rmsd)
    : m_path(RESOURCE_PATH() + path)
    , m_max_rmsd(max_rmsd)
    , m_matcher(m_path)
    , m_subobject_area_ratio(0)
{}

SubObjectTemplateMatcher::SubObjectTemplateMatcher(const char* path, Color background_replacement, double max_rmsd)
    : SubObjectTemplateMatcher(path, max_rmsd)
{
    m_background_replacement = background_replacement;
}

ImagePixelBox SubObjectTemplateMatcher::object_from_subobject(
    size_t width, size_t height,
    const ImagePixelBox& subobject_in_image
) const{
    return extract_object_from_inner_feature(
        width, height,
        subobject_in_image, m_subobject_in_object_f
    );
}

double SubObjectTemplateMatcher::rmsd(
    ImagePixelBox& object_box,
    const ImageViewRGB32& image, const ImagePixelBox& subobject_in_image
) const{
    object_box = object_from_subobject(image.width(), image.height(), subobject_in_image);
//    cout << object_box.min_x << ", "
//         << object_box.min_y << ", "
//         << object_box.max_x << ", "
//         << object_box.max_y << endl;
    ImageViewRGB32 object = extract_box_reference(image, object_box);

//    object.save("test.png");


    if (!object || !check_image(object)){
        return 99999.;
    }

    return m_matcher.rmsd(object);
}
double SubObjectTemplateMatcher::rmsd_with_background_replace(
    ImagePixelBox& object_box,
    const ImageViewRGB32& image, const PackedBinaryMatrix& binary_image,
    const ImagePixelBox& subobject_in_image
) const{
    object_box = object_from_subobject(image.width(), image.height(), subobject_in_image);
    ImageRGB32 object = extract_box_reference(image, object_box).copy();
    if (!object || !check_image(object)){
        return 99999.;
    }

//    size_t width = binary_image.width();
//    size_t height = binary_image.height();
//    cout << width << " x " << height
//         << ": " << object_box.min_x << "-" << object_box.width()
//         << " " << object_box.min_y << "-" << object_box.height() << endl;

    filter_by_mask(
        binary_image.submatrix(
            object_box.min_x, object_box.min_y,
            object_box.width(), object_box.height()
        ),
        object,
        m_background_replacement,
        true
    );

    return m_matcher.rmsd(object);
}


void SubObjectTemplateMatcher::set_subobject(const WaterfillObject& subobject_in_object){
    m_subobject_in_object_p = subobject_in_object;
    m_subobject_in_object_f = pixelbox_to_floatbox(m_matcher.image_template(), m_subobject_in_object_p);
    m_subobject_area_ratio = subobject_in_object.area_ratio();
}
bool SubObjectTemplateMatcher::check_aspect_ratio(size_t candidate_width, size_t candidate_height) const{
//    double expected_aspect_ratio = (double)m_subobject_in_object_p.width() / m_subobject_in_object_p.height();
//    double actual_aspect_ratio = (double)width / height;
//    double error = actual_aspect_ratio / expected_aspect_ratio;

//    cout << "expected_aspect_ratio = " << expected_aspect_ratio << endl;
//    cout << "actual_aspect_ratio = " << actual_aspect_ratio << endl;

    double error = (double)m_subobject_in_object_p.width() * candidate_height;
    error /= (double)m_subobject_in_object_p.height() * candidate_width;
//    cout << "ratio = " << error << endl;
    return m_aspect_ratio_lower <= error && error <= m_aspect_ratio_upper;
}
bool SubObjectTemplateMatcher::check_area_ratio(double candidate_area_ratio) const{
    if (m_subobject_area_ratio == 0){
        return true;
    }
    double error = candidate_area_ratio / m_subobject_area_ratio;
    return m_area_ratio_lower <= error && error <= m_area_ratio_upper;
}

bool SubObjectTemplateMatcher::matches(
    ImagePixelBox& object_box,
    const ImageViewRGB32& image,
    const WaterfillObject& subobject_in_image
) const{
    if (!check_aspect_ratio(subobject_in_image.width(), subobject_in_image.height())){
        return false;
    }
    if (!check_area_ratio(subobject_in_image.area_ratio())){
        return false;
    }

//    static int c = 0;
//    cout << c++ << endl;

    double rmsd = this->rmsd(object_box, image, subobject_in_image);

//    cout << "rmsd  = " << rmsd << endl;

//    if (rmsd <= m_max_rmsd){
//        static int c = 0;
//        extract_box(image, object_box).save("test-" + std::to_string(c++) + "-" + std::to_string(rmsd) + ".png");
//    }

    return rmsd <= m_max_rmsd;
}
bool SubObjectTemplateMatcher::matches_with_background_replace(
    ImagePixelBox& object_box,
    const ImageViewRGB32& image, const PackedBinaryMatrix& binary_image,
    const WaterfillObject& subobject_in_image
) const{
    if (!check_aspect_ratio(subobject_in_image.width(), subobject_in_image.height())){
        return false;
    }
    if (!check_area_ratio(subobject_in_image.area_ratio())){
        return false;
    }

    double rmsd = this->rmsd_with_background_replace(object_box, image, binary_image, subobject_in_image);
//    cout << "rmsd  = " << rmsd << endl;
    return rmsd <= m_max_rmsd;
}



}
}
