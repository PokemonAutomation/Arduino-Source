/*  Black Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Returns true after a black screen has been detected and
 * the black screen has ended.
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "BlackScreenDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

BlackScreenDetector::BlackScreenDetector(
    VideoFeed& feed, Logger& logger
)
    : m_feed(feed)
    , m_logger(logger)
    , m_box(feed, 0.0, 0.0, 1.0, 1.0)
    , m_has_been_black(false)
{}
BlackScreenDetector::BlackScreenDetector(
    VideoFeed& feed, Logger& logger,
    const InferenceBox& box
)
    : m_feed(feed)
    , m_logger(logger)
    , m_box(feed, box)
    , m_has_been_black(false)
{}


bool BlackScreenDetector::black_is_over(){
    //  Grab baseline image.
    QImage image = m_feed.snapshot();
    if (image.isNull()){
        m_logger.log("BlackScreenDetector(): Screenshot failed.", "purple");
        return false;
    }

    ImageStats stats = pixel_stats(image);
    double average = stats.average.sum();
    double stddev = stats.stddev.sum();
//    cout << stats.average << endl;
    m_logger.log("BlackScreenDetector(): a = " + QString::number(average) + ", s = " + QString::number(stddev), "purple");
    if (average < 100 && stddev < 10){
        m_has_been_black = true;
        return false;
    }
    return m_has_been_black;
}




}
