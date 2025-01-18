/*  Black Border Check
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonTools_BlackBorderCheck_H
#define PokemonAutomation_CommonTools_BlackBorderCheck_H

#include "CommonFramework/Globals.h"

namespace PokemonAutomation{

class VideoStream;


void start_program_video_check(VideoStream& stream, FeedbackType feedback);


}
#endif
