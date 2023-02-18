/*  Video Resolution Check
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_VideoResolutionCheck_H
#define PokemonAutomation_VideoResolutionCheck_H

namespace PokemonAutomation{

class Logger;
class ImageViewRGB32;
class ConsoleHandle;


void assert_16_9_720p_min(Logger& logger, const ImageViewRGB32& frame);
void assert_16_9_720p_min(Logger& logger, ConsoleHandle& console);

void assert_16_9_1080p_min(Logger& logger, const ImageViewRGB32& frame);
void assert_16_9_1080p_min(Logger& logger, ConsoleHandle& console);



}
#endif
