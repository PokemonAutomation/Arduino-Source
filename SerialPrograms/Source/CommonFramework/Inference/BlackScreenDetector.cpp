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

BlackScreenDetector::BlackScreenDetector()
    : m_box(0.1, 0.1, 0.8, 0.8)
{
    add_box(m_box);
}
bool BlackScreenDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    QImage image = extract_box(frame, m_box);
    return is_black(image);
}




BlackScreenOverDetector::BlackScreenOverDetector()
    : m_box(0.1, 0.1, 0.8, 0.8)
    , m_has_been_black(false)
{
    add_box(m_box);
}
BlackScreenOverDetector::BlackScreenOverDetector(const ImageFloatBox& box)
    : m_box(box)
    , m_has_been_black(false)
{
    add_box(m_box);
}


bool BlackScreenOverDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return black_is_over(frame);
}
bool BlackScreenOverDetector::black_is_over(const QImage& frame){
    QImage image = extract_box(frame, m_box);
    if (is_black(image)){
        m_has_been_black = true;
        return false;
    }
    return m_has_been_black;
}



}
