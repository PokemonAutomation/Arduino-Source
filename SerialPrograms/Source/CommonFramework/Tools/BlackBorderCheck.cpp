/*  Black Border Check
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Cpp/Exception.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Inference/BlackBorderDetector.h"
#include "BlackBorderCheck.h"

namespace PokemonAutomation{


void start_program_video_check(ConsoleHandle& console, FeedbackType feedback){
    if (feedback == FeedbackType::NONE){
        return;
    }

    BlackBorderDetector detector;
    VideoOverlaySet set(console);
    detector.make_overlays(set);

    QImage screen = console.video().snapshot();
    if (screen.isNull() && feedback == FeedbackType::OPTIONAL_){
        return;
    }

    if (detector.detect(screen)){
        console.log("Black border detected! Please set your screen size to 100%.", COLOR_RED);
        PA_THROW_StringException("Black border detected! Please set your screen size to 100%.");
    }
}
void start_program_video_check(FixedLimitVector<ConsoleHandle>& consoles, FeedbackType feedback){
    for (ConsoleHandle& console : consoles){
        start_program_video_check(console, feedback);
    }
}



}
