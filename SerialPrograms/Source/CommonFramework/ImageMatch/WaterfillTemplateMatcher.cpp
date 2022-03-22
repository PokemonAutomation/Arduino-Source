/*  Waterfill Template Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/ImageOpener.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "WaterfillTemplateMatcher.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{

using namespace Kernels;
using namespace Kernels::Waterfill;


WaterfillTemplateMatcher::WaterfillTemplateMatcher(
    const char* path,
    Color min_color, Color max_color,
    size_t min_area
){
    QString qpath = RESOURCE_PATH() + path;
    QImage reference = open_image(qpath);

    PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_range(reference, (uint32_t)min_color, (uint32_t)max_color);
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, min_area, false);
    if (objects.empty()){
        throw FileException(
            nullptr, PA_CURRENT_FUNCTION,
            "Failed to find exactly any objects in resource.",
            qpath.toStdString()
        );
    }

    const WaterfillObject* best = &objects[0];
    for (const WaterfillObject& object : objects){
        if (best->area < object.area){
            best = &object;
        }
    }

    m_object = extract_box(reference, *best);
//    m_object.save("test.png");
    m_matcher.reset(new ExactImageMatcher(m_object));
    m_area_ratio = best->area_ratio();
}

double WaterfillTemplateMatcher::rmsd(const QImage& object) const{
    if (object.isNull() || !check_image(object)){
        return 99999.;
    }
    return m_matcher->rmsd(object);
}
bool WaterfillTemplateMatcher::check_aspect_ratio(size_t candidate_width, size_t candidate_height) const{
//    double expected_aspect_ratio = (double)m_subobject_in_object_p.width() / m_subobject_in_object_p.height();
//    double actual_aspect_ratio = (double)width / height;
//    double error = actual_aspect_ratio / expected_aspect_ratio;

//    cout << "expected_aspect_ratio = " << expected_aspect_ratio << endl;
//    cout << "actual_aspect_ratio = " << actual_aspect_ratio << endl;

    double error = (double)m_object.width() * candidate_height;
    error /= (double)m_object.height() * candidate_width;
//    cout << "ratio = " << error << endl;
    return m_aspect_ratio_lower <= error && error <= m_aspect_ratio_upper;
}
bool WaterfillTemplateMatcher::check_area_ratio(double candidate_area_ratio) const{
    if (m_area_ratio == 0){
        return true;
    }
    double error = candidate_area_ratio / m_area_ratio;
//    cout << "area = " << error << endl;
    return m_area_ratio_lower <= error && error <= m_area_ratio_upper;
}
double WaterfillTemplateMatcher::rmsd_precropped(const QImage& cropped_image, const WaterfillObject& object) const{
    if (!check_aspect_ratio(object.width(), object.height())){
//        cout << "bad aspect ratio" << endl;
        return 99999.;
    }
    if (!check_area_ratio(object.area_ratio())){
//        cout << "bad area ratio" << endl;
        return 99999.;
    }

//    static int c = 0;
//    cout << c << endl;

    double rmsd = this->rmsd(cropped_image);

//    cout << "rmsd  = " << rmsd << endl;

//    if (rmsd <= m_max_rmsd){
//        static int c = 0;
//        cropped_image.save("test-" + QString::number(c++) + "-" + QString::number(rmsd) + ".png");
//    }

    return rmsd;
}
double WaterfillTemplateMatcher::rmsd_original(const QImage& original_image, const WaterfillObject& object) const{
    if (!check_aspect_ratio(object.width(), object.height())){
//        cout << "bad aspect ratio" << endl;
        return 99999.;
    }
    if (!check_area_ratio(object.area_ratio())){
//        cout << "bad area ratio" << endl;
        return 99999.;
    }

//    static int c = 0;
//    cout << c << endl;

    double rmsd = this->rmsd(extract_box(original_image, object));

//    cout << "rmsd  = " << rmsd << endl;

//    if (rmsd <= m_max_rmsd){
//        static int c = 0;
//        extract_box(image, object).save("test-" + QString::number(c++) + "-" + QString::number(rmsd) + ".png");
//    }

    return rmsd;
}




}
}
