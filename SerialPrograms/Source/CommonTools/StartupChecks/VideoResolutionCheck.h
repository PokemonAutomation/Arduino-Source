/*  Video Resolution Check
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_VideoResolutionCheck_H
#define PokemonAutomation_CommonTools_VideoResolutionCheck_H

namespace PokemonAutomation{

class Logger;
class ImageViewRGB32;
class VideoFeed;


void assert_16_9_720p_min(Logger& logger, const ImageViewRGB32& frame);
void assert_16_9_720p_min(Logger& logger, VideoFeed& video);

void assert_16_9_1080p_min(Logger& logger, const ImageViewRGB32& frame);
void assert_16_9_1080p_min(Logger& logger, VideoFeed& video);



}
#endif
