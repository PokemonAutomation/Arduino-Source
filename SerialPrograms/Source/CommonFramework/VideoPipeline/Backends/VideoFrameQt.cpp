/*  Video Frame
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVideoFrameFormat>
#include "VideoFrameQt.h"

namespace PokemonAutomation{


VideoFormat QVideoFrameFormat_to_VideoFormat(QVideoFrameFormat::PixelFormat format){
    switch (format){
    case QVideoFrameFormat::Format_YUV420P:
        return VideoFormat::YUV420P;
    case QVideoFrameFormat::Format_YUYV:
        return VideoFormat::YUYV;
    case QVideoFrameFormat::Format_NV12:
        return VideoFormat::NV12;
    case QVideoFrameFormat::Format_P010:
        return VideoFormat::P010;
    case QVideoFrameFormat::Format_Jpeg:
        return VideoFormat::MJPEG;
    default:
        return VideoFormat::OTHER;
    }
}
QVideoFrameFormat::PixelFormat VideoFormat_to_QVideoFrameFormat(VideoFormat format){
    switch (format){
    case VideoFormat::OTHER:
        return QVideoFrameFormat::Format_Invalid;
    case VideoFormat::YUV420P:
        return QVideoFrameFormat::Format_YUV420P;
    case VideoFormat::YUYV:
        return QVideoFrameFormat::Format_YUYV;
    case VideoFormat::NV12:
        return QVideoFrameFormat::Format_NV12;
    case VideoFormat::P010:
        return QVideoFrameFormat::Format_P010;
    case VideoFormat::MJPEG:
        return QVideoFrameFormat::Format_Jpeg;
    }
    return QVideoFrameFormat::Format_Invalid;
}



}
