/*  Image Match Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "ImageMatchDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



ImageMatchDetector::ImageMatchDetector(QImage reference_image, const ImageFloatBox& box, double max_rmsd)
    : m_reference_image(std::move(reference_image))
    , m_box(box)
    , max_rmsd(max_rmsd)
{
    add_box(m_box);
    m_reference_image = extract_box(m_reference_image, m_box);
}

bool ImageMatchDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point
){
    if (frame.isNull()){
        return false;
    }
    QImage scaled = extract_box(frame, m_box);
    if (scaled.size() != m_reference_image.size()){
        scaled = scaled.scaled(m_reference_image.size());
    }
    double rmsd = ImageMatch::pixel_RMSD(m_reference_image, scaled);
//    cout << rmsd << endl;
    return rmsd <= max_rmsd;
}






}
