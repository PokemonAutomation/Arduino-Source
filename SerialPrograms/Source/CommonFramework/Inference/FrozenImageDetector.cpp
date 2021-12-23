/*  Frozen Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "FrozenImageDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


FrozenImageDetector::FrozenImageDetector(std::chrono::milliseconds timeout, double rmsd_threshold)
    : m_timeout(timeout)
    , m_rmsd_threshold(rmsd_threshold)
{}
void FrozenImageDetector::make_overlays(OverlaySet&) const{

}
bool FrozenImageDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    if (m_last_delta.size() != frame.size()){
        m_timestamp = timestamp;
        m_last_delta = frame;
        return false;
    }

    double rmsd = ImageMatch::pixel_RMSD(m_last_delta, frame);
//    cout << "rmsd = " << rmsd << endl;
    if (rmsd > m_rmsd_threshold){
        m_timestamp = timestamp;
        m_last_delta = frame;
        return false;
    }

    return timestamp - m_timestamp > m_timeout;
//    return false;
}


}
