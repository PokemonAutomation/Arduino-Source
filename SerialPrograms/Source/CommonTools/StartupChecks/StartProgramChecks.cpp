/*  Start Program Checks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
//#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/VisualDetectors/BlackBorderDetector.h"
#include "Controllers/ControllerTypes.h"
#include "Controllers/Controller.h"
#include "StartProgramChecks.h"

namespace PokemonAutomation{
namespace StartProgramChecks{


void check_feedback(VideoStream& stream, FeedbackType feedback){
    if (feedback == FeedbackType::NONE){
        return;
    }
    VideoSnapshot screen = stream.video().snapshot();
    if (screen){
        return;
    }
    if (feedback == FeedbackType::REQUIRED || feedback == FeedbackType::VIDEO_AUDIO){
        throw UserSetupError(
            stream.logger(),
            "This program requires video feedback. Please make sure the video is working."
        );
    }
}

void check_border(VideoStream& stream){
    BlackBorderDetector detector;
    VideoOverlaySet set(stream.overlay());
    detector.make_overlays(set);
    VideoSnapshot screen = stream.video().snapshot();
    if (!detector.detect(screen)){
        return;
    }
    throw UserSetupError(
        stream.logger(),
        "Black border detected! Please set your screen size to 100% in the TV Settings on your Nintendo Switch."
    );
}



void check_performance_class_wired_or_wireless(AbstractController& controller){
    switch (controller.performance_class()){
    case ControllerPerformanceClass::SerialPABotBase_Wired:
    case ControllerPerformanceClass::SerialPABotBase_Wireless:
        return;
    default:
        throw UserSetupError(
            controller.logger(),
            "Incompatible Controller:\n\n"
            "This program requires a controller with performance class \"Wired\" or \"Wireless\".\n\n"
            "If you are using sys-botbase 2, please upgrade to sys-botbase 3."
        );
    }
}



}
}
