/*  Receive Pokemon (Orange Background) Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_ReceivePokemonDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ReceivePokemonDetector::ReceivePokemonDetector(
    VideoFeed& feed
)
    : m_box(feed, 0.2, 0.1, 0.6, 0.1)
    , m_has_been_orange(false)
{}
ReceivePokemonDetector::ReceivePokemonDetector(
    VideoFeed& feed,
    const InferenceBox& box
)
    : m_box(feed, box)
    , m_has_been_orange(false)
{}


bool ReceivePokemonDetector::on_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return receive_is_over(frame);
}
bool ReceivePokemonDetector::receive_is_over(const QImage& frame){
    QImage image = extract_box(frame, m_box);
//    QImage image = m_feed.snapshot();
//    if (image.isNull()){
//        m_logger.log("BlackScreenDetector(): Screenshot failed.", "purple");
//        return false;
//    }

//    ImageStats stats = pixel_stats(image);
//    double average = stats.average.sum();
//    double stddev = stats.stddev.sum();
//    cout << stats.average << endl;
//    m_logger.log("BlackScreenDetector(): a = " + QString::number(average) + ", s = " + QString::number(stddev), "purple");
//    if (average < 100 && stddev < 10){


    ImageStats stats = pixel_stats(image);
    FloatPixel expected(193, 78, 56);
    FloatPixel actual = stats.average;

    expected /= expected.sum();
    actual /= actual.sum();

    double distance = euclidean_distance(expected, actual);

//    cout << "average = " << stats.average << ", stddev = " << stats.stddev << ", distance = " << distance << endl;
//    cout << "m_has_been_orange = " << m_has_been_orange << endl;

//    double average = stats.average.sum();
//    double stddev = stats.stddev.sum();
//    return average <= max_rgb_sum && stddev <= max_stddev_sum;


    if (stats.stddev.sum() < 10 && distance < 0.2){
        m_has_been_orange = true;
        return false;
    }
    return m_has_been_orange;
}


}
}
}
