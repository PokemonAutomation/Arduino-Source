/*  Video Frame
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVideoFrameFormat>
#include "Kernels/YUVConversion/Kernels_YUVConversion.h"
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



ImageRGB32 frame_to_ImageRGB32(const QVideoFrame& frame, bool force_full_range){
    using namespace Kernels;

    const QVideoFrameFormat format = frame.surfaceFormat();
    const QVideoFrameFormat::PixelFormat pixel_format = format.pixelFormat();

    switch (pixel_format){
    case QVideoFrameFormat::Format_YUYV:
    case QVideoFrameFormat::Format_UYVY:
    case QVideoFrameFormat::Format_YUV420P:
    case QVideoFrameFormat::Format_YV12:
    case QVideoFrameFormat::Format_NV12:
    case QVideoFrameFormat::Format_NV21:
        break;
    default:
        return ImageRGB32();
    }

    const int width = frame.width();
    const int height = frame.height();
    //  All of these subsample chroma 2:1 horizontally.
    if (width <= 0 || height <= 0 || (width & 1) != 0){
        return ImageRGB32();
    }

    YUVColorSpace color_space;
    switch (format.colorSpace()){
    case QVideoFrameFormat::ColorSpace_BT709:
        color_space = YUVColorSpace::BT709;
        break;
    case QVideoFrameFormat::ColorSpace_BT601:
        color_space = YUVColorSpace::BT601;
        break;
    default:
        //  Undeclared: BT.601 for SD, BT.709 for HD.
        color_space = height > 576 ? YUVColorSpace::BT709 : YUVColorSpace::BT601;
        break;
    }
    const YUVColorRange color_range =
        force_full_range || format.colorRange() == QVideoFrameFormat::ColorRange_Full
            ? YUVColorRange::Full
            : YUVColorRange::Limited;
    const YUVCoefficients coefficients = yuv_coefficients(color_space, color_range);

    QVideoFrame source = frame;
    if (!source.map(QVideoFrame::ReadOnly)){
        return ImageRGB32();
    }

    ImageRGB32 image((size_t)width, (size_t)height);
    uint32_t* dst = image.data();
    const size_t dst_bytes_per_row = image.bytes_per_row();

    switch (pixel_format){
    case QVideoFrameFormat::Format_YUYV:
        convert_YUY2_to_RGB32(
            (size_t)width, (size_t)height,
            source.bits(0), (size_t)source.bytesPerLine(0),
            dst, dst_bytes_per_row, coefficients
        );
        break;
    case QVideoFrameFormat::Format_UYVY:
        convert_UYVY_to_RGB32(
            (size_t)width, (size_t)height,
            source.bits(0), (size_t)source.bytesPerLine(0),
            dst, dst_bytes_per_row, coefficients
        );
        break;
    case QVideoFrameFormat::Format_YUV420P:
    case QVideoFrameFormat::Format_YV12:{
        //  YV12 is YUV420P with the two chroma planes swapped.
        const int u = pixel_format == QVideoFrameFormat::Format_YV12 ? 2 : 1;
        const int v = pixel_format == QVideoFrameFormat::Format_YV12 ? 1 : 2;
        convert_YUV420P_to_RGB32(
            (size_t)width, (size_t)height,
            source.bits(0), (size_t)source.bytesPerLine(0),
            source.bits(u), (size_t)source.bytesPerLine(u),
            source.bits(v), (size_t)source.bytesPerLine(v),
            dst, dst_bytes_per_row, coefficients
        );
        break;
    }
    case QVideoFrameFormat::Format_NV12:
    case QVideoFrameFormat::Format_NV21:
        convert_NV12_to_RGB32(
            (size_t)width, (size_t)height,
            source.bits(0), (size_t)source.bytesPerLine(0),
            source.bits(1), (size_t)source.bytesPerLine(1),
            pixel_format == QVideoFrameFormat::Format_NV21,
            dst, dst_bytes_per_row, coefficients
        );
        break;
    default:
        break;
    }

    source.unmap();
    return image;
}


}
