/*  Black Border Check
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
//#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/VisualDetectors/BlackBorderDetector.h"
#include "BlackBorderCheck.h"

namespace PokemonAutomation{


void start_program_video_check(VideoStream& stream, FeedbackType feedback){
    if (feedback == FeedbackType::NONE){
        return;
    }

    VideoSnapshot screen = stream.video().snapshot();

    if (!screen){
        if (feedback == FeedbackType::REQUIRED || feedback == FeedbackType::VIDEO_AUDIO){
            throw UserSetupError(stream.logger(), "This program requires video feedback. Please make sure the video is working.");
        }
        return;
    }

    BlackBorderDetector detector;
    VideoOverlaySet set(stream.overlay());
    detector.make_overlays(set);

    if (detector.detect(screen)){
        throw UserSetupError(stream.logger(), "Black border detected! Please set your screen size to 100% in the TV Settings on your Nintendo Switch.");
    }
}



}
