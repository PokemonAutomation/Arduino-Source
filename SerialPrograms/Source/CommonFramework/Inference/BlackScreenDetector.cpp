/*  Black Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Returns true after a black screen has been detected and
 * the black screen has ended.
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "BlackScreenDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

BlackScreenDetector::BlackScreenDetector(
    VideoOverlay& overlay
)
    : m_box(overlay, 0.1, 0.1, 0.8, 0.8)
    , m_has_been_black(false)
{}
BlackScreenDetector::BlackScreenDetector(
    VideoOverlay& overlay,
    const ImageFloatBox& box
)
    : m_box(overlay, box)
    , m_has_been_black(false)
{}


bool BlackScreenDetector::on_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return black_is_over(frame);
}
bool BlackScreenDetector::black_is_over(const QImage& frame){
    QImage image = extract_box(frame, m_box);
    if (is_black(image)){
        m_has_been_black = true;
        return false;
    }
    return m_has_been_black;
}



}
