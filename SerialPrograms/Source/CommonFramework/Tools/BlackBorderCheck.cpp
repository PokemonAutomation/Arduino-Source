/*  Black Border Check
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Inference/BlackBorderDetector.h"
#include "CommonFramework/Inference/BlackBorderGBADetector.h"
#include "BlackBorderCheck.h"

namespace PokemonAutomation{


void start_program_video_check(ConsoleHandle& console, FeedbackType feedback){
    if (feedback == FeedbackType::NONE){
        return;
    }

    VideoSnapshot screen = console.video().snapshot();

    if (!screen){
        if (feedback == FeedbackType::REQUIRED || feedback == FeedbackType::VIDEO_AUDIO || feedback == FeedbackType::VIDEO_AUDIO_GBA){
            throw UserSetupError(console, "This program requires video feedback. Please make sure the video is working.");
        }
        return;
    }

    if (feedback != FeedbackType::VIDEO_AUDIO_GBA) { //GB, GBC in fullscreen will reach the top and bottom of the screen
        BlackBorderDetector detector;
        VideoOverlaySet set(console);
        detector.make_overlays(set);

        if (detector.detect(screen)) {
            throw UserSetupError(console, "Black border detected! Please set your screen size to 100% in the TV Settings on your Nintendo Switch.");
        }
    }
    else {
        BlackBorderGBADetector detector;
        VideoOverlaySet set(console);
        detector.make_overlays(set);

        if (detector.detect(screen)) {
            throw UserSetupError(console, "Black border detected! Please set your screen size to 100% in the TV Settings on your Nintendo Switch.");
        }
    }
}
void start_program_video_check(FixedLimitVector<ConsoleHandle>& consoles, FeedbackType feedback){
    for (ConsoleHandle& console : consoles){
        start_program_video_check(console, feedback);
    }
}



}
