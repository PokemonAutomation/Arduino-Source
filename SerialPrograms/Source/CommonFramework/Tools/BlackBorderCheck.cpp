/*  Black Border Check
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/VideoPipeline/VideoOverlaySet.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Inference/BlackBorderDetector.h"
#include "BlackBorderCheck.h"

namespace PokemonAutomation{


void start_program_video_check(ConsoleHandle& console, FeedbackType feedback){
    if (feedback == FeedbackType::NONE){
        return;
    }

    QImage screen = console.video().snapshot();

    if (screen.isNull()){
        if (feedback == FeedbackType::REQUIRED){
            throw UserSetupError(console, "This program requires video feedback. Please make sure the video is working.");
        }
        return;
    }

    BlackBorderDetector detector;
    VideoOverlaySet set(console);
    detector.make_overlays(set);

    if (detector.detect(screen)){
        throw UserSetupError(console, "Black border detected! Please set your screen size to 100%.");
    }
}
void start_program_video_check(FixedLimitVector<ConsoleHandle>& consoles, FeedbackType feedback){
    for (ConsoleHandle& console : consoles){
        start_program_video_check(console, feedback);
    }
}



}
