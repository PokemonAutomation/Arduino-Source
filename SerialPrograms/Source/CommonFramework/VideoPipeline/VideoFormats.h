/*  Video Formats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_VideoFormats_H
#define PokemonAutomation_VideoFormats_H

#include "Common/Cpp/Options/EnumDropdownDatabase.h"

namespace PokemonAutomation{


//  These should be ordered by preference.
enum class VideoFormat{
    P010,
    NV12,
    YUV420P,
    YUYV,
    MJPEG,
    OTHER,
};

const EnumDropdownDatabase<VideoFormat>& VideoFormat_database();


using FramesPerSecond = size_t;



}
#endif
