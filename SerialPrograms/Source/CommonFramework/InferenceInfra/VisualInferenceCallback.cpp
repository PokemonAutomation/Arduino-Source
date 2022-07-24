/*  Visual Inference Callback
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "VisualInferenceCallback.h"

namespace PokemonAutomation{


bool VisualInferenceCallback::process_frame(const std::shared_ptr<const ImageRGB32>& frame, WallClock timestamp){
    return process_frame(*frame, timestamp);
}
bool VisualInferenceCallback::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "You must override one of the two process_frame() functions.");
}





}
