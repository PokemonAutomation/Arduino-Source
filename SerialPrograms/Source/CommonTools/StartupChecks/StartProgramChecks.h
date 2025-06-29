/*  Start Program Checks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_StartProgramChecks_H
#define PokemonAutomation_CommonTools_StartProgramChecks_H

#include <set>
#include "CommonFramework/Globals.h"

namespace PokemonAutomation{
    class Logger;
    class VideoStream;
    class ControllerFeatures;
namespace StartProgramChecks{



void check_feedback(VideoStream& stream, FeedbackType feedback);
void check_border(VideoStream& stream);

void check_controller_features(
    Logger& logger,
    const ControllerFeatures& capabilities,
    const ControllerFeatures& required_features
);



}
}
#endif
