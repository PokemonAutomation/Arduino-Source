/*  Sub-Object Template Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/Globals.h"
#include "SubObjectTemplateMatcher.h"

namespace PokemonAutomation{
namespace ImageMatch{


SubObjectTemplateMatcher::SubObjectTemplateMatcher(const char* path, double max_rmsd)
    : m_object(RESOURCE_PATH() + path)
    , m_max_rmsd(max_rmsd)
    , m_aspect_ratio_min(0)
    , m_aspect_ratio_max(0)
    , m_matcher(m_object)
{
    if (m_object.isNull()){
        PA_THROW_StringException(std::string("Unable to Open: ") + path);
    }
}

ImagePixelBox SubObjectTemplateMatcher::object_from_subobject(const ImagePixelBox& subobject_in_image) const{
    return extract_object_from_inner_feature(subobject_in_image, m_subobject_in_object_f);
}

double SubObjectTemplateMatcher::rmsd(
    ImagePixelBox& object_box,
    const QImage& image, const ImagePixelBox& subobject_in_image
) const{
    object_box = object_from_subobject(subobject_in_image);
//    cout << object_box.min_x << ", "
//         << object_box.min_y << ", "
//         << object_box.max_x << ", "
//         << object_box.max_y << endl;
    QImage object = extract_box(image, object_box);
//    object.save("test.png");
    if (object.isNull()){
        return 99999.;
    }
    return m_matcher.rmsd(object);
}


void SubObjectTemplateMatcher::set_subobject(const ImagePixelBox& subobject_in_object){
    m_subobject_in_object_p = subobject_in_object;
    m_subobject_in_object_f = pixelbox_to_floatbox(m_object, m_subobject_in_object_p);
}
bool SubObjectTemplateMatcher::check_aspect_ratio(size_t width, size_t height) const{
//    double expected_aspect_ratio = (double)m_subobject_in_object_p.width() / m_subobject_in_object_p.height();
//    double actual_aspect_ratio = (double)width / height;
//    double error = actual_aspect_ratio / expected_aspect_ratio;

//    cout << "expected_aspect_ratio = " << expected_aspect_ratio << endl;
//    cout << "actual_aspect_ratio = " << actual_aspect_ratio << endl;

    double error = (double)m_subobject_in_object_p.width() * height;
    error /= (double)m_subobject_in_object_p.height() * width;
//    cout << "ratio = " << error << endl;
    return 0.8 < error && error < 1.25;
}

bool SubObjectTemplateMatcher::matches(
    ImagePixelBox& object_box,
    const QImage& image, const ImagePixelBox& subobject_in_image
) const{
    //  Check aspect ratio.
    if (!check_aspect_ratio(subobject_in_image.width(), subobject_in_image.height())){
        return false;
    }

    double rmsd = this->rmsd(object_box, image, subobject_in_image);
//    cout << "rmsd  = " << rmsd << endl;

//    if (rmsd <= m_max_rmsd){
//        static int c = 0;
//        extract_box(image, object_box).save("test-" + QString::number(c++) + "-" + QString::number(rmsd) + ".png");
//    }

    return rmsd <= m_max_rmsd;
}



}
}
