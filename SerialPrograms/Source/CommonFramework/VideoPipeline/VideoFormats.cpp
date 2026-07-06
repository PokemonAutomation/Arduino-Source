/*  Video Formats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "VideoFormats.h"

namespace PokemonAutomation{



const EnumDropdownDatabase<VideoFormat>& VideoFormat_database(){
    static EnumDropdownDatabase<VideoFormat> database{
        {VideoFormat::OTHER,    "other",    "Unknown"},
        {VideoFormat::YUV420P,  "YUV420P",  "YUV420P"},
        {VideoFormat::YUYV,     "YUYV",     "YUYV"},
        {VideoFormat::NV12,     "NV12",     "NV12"},
        {VideoFormat::P010,     "P010",     "P010"},
        {VideoFormat::MJPEG,    "MJPEG",    "MJPEG"},
    };
    return database;
}



}
