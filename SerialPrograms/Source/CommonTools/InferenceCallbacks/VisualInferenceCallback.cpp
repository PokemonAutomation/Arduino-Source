/*  Visual Inference Callback
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "VisualInferenceCallback.h"

namespace PokemonAutomation{


bool VisualInferenceCallback::process_frame(const VideoSnapshot& frame){
    return process_frame(*frame.frame, frame.timestamp);
}
bool VisualInferenceCallback::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "You must override one of the two process_frame() functions.");
}





}
