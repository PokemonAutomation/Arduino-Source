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



ImageMatchDetector::ImageMatchDetector(
    QImage reference_image, const ImageFloatBox& box,
    double max_rmsd,
    std::chrono::milliseconds hold_duration
)
    : m_reference_image(std::move(reference_image))
    , m_box(box)
    , m_max_rmsd(max_rmsd)
    , m_hold_duration(hold_duration)
    , m_last_match(false)
    , m_start_of_match(std::chrono::system_clock::time_point::min())
{
    add_box(m_box);
    m_reference_image = extract_box(m_reference_image, m_box);
}

double ImageMatchDetector::rmsd(const QImage& frame){
    if (frame.isNull()){
        return 1000;
    }
    QImage scaled = extract_box(frame, m_box);
    if (scaled.size() != m_reference_image.size()){
        scaled = scaled.scaled(m_reference_image.size());
    }
//    cout << "asdf" << endl;
    double ret = ImageMatch::pixel_RMSD(m_reference_image, scaled);
//    cout << "rmsd = " << ret << endl;
    return ret;
}
bool ImageMatchDetector::matches(const QImage& frame){
    return rmsd(frame) <= m_max_rmsd;
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
    if (rmsd > m_max_rmsd){
        return false;
    }
    auto now = std::chrono::system_clock::now();
    if (!m_last_match){
        m_last_match = true;
        m_start_of_match = now;
        return false;
    }
    return now - m_start_of_match >= m_hold_duration;
}






}
