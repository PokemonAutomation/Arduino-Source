/*  Black Border Check
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_BlackBorderCheck_H
#define PokemonAutomation_CommonFramework_BlackBorderCheck_H

#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/Globals.h"

namespace PokemonAutomation{

class VideoStream;
class ConsoleHandle;


void start_program_video_check(VideoStream& stream, FeedbackType feedback);
void start_program_video_check(FixedLimitVector<ConsoleHandle>& consoles, FeedbackType feedback);


}
#endif
