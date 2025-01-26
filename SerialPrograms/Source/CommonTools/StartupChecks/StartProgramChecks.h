/*  Start Program Checks
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonTools_StartProgramChecks_H
#define PokemonAutomation_CommonTools_StartProgramChecks_H

#include "CommonFramework/Globals.h"

namespace PokemonAutomation{
    class VideoStream;
namespace StartProgramChecks{



void check_feedback(VideoStream& stream, FeedbackType feedback);
void check_border(VideoStream& stream);



}
}
#endif
