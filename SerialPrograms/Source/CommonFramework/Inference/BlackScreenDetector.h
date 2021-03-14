/*  Black Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Returns true after a black screen has been detected and
 * the black screen has ended.
 *
 */

#ifndef PokemonAutomation_CommonFramework_BlackScreenDetector_H
#define PokemonAutomation_CommonFramework_BlackScreenDetector_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/Logger.h"

namespace PokemonAutomation{


class BlackScreenDetector{
public:
    BlackScreenDetector(
        VideoFeed& feed, Logger& logger
    );
    BlackScreenDetector(
        VideoFeed& feed, Logger& logger,
        const InferenceBox& box
    );

    bool black_is_over();


private:
    VideoFeed& m_feed;
    Logger& m_logger;
    InferenceBoxScope m_box;
    bool m_has_been_black;
};



}
#endif
