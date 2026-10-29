/*  Video Resolution Check
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "VideoResolutionCheck.h"

namespace PokemonAutomation{


void assert_16_9_720p_min(Logger& logger, const ImageViewRGB32& frame){
    if (!frame){
        throw UserSetupError(logger, "No video detected.");
    }
    if (frame.height() < 720){
        throw UserSetupError(logger, "Video resolution must be at least 720p.");
    }
    double aspect_ratio = (double)frame.width() / frame.height();
    if (aspect_ratio < 1.77 || aspect_ratio > 1.78){
        throw UserSetupError(logger, "Video aspect ratio must be 16:9.");
    }
}
void assert_16_9_720p_min(Logger& logger, VideoFeed& video){
    assert_16_9_720p_min(logger, video.snapshot());
}

void assert_16_9_1080p_min(Logger& logger, const ImageViewRGB32& frame){
    if (!frame){
        throw UserSetupError(logger, "No video detected.");
    }
    if (frame.height() < 1080){
        throw UserSetupError(logger, "Video resolution must be at least 1080p.");
    }
    double aspect_ratio = (double)frame.width() / frame.height();
    if (aspect_ratio < 1.77 || aspect_ratio > 1.78){
        throw UserSetupError(logger, "Video aspect ratio must be 16:9.");
    }
}
void assert_16_9_1080p_min(Logger& logger, VideoFeed& video){
    assert_16_9_1080p_min(logger, video.snapshot());
}



}
