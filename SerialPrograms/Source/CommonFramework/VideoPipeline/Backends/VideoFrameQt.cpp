/*  Video Frame
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVideoFrameFormat>
#include "Common/Cpp/Color.h"
#include <cstring>
#include <algorithm>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/VideoPipelineOptions.h"
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



//  Capture backends (gstreamer in particular) hand out buffer pool frames.
//  Retaining one keeps its buffer checked out and the pool allocates another,
//  so memory grows without bound. Copy any frame that will be retained.
QVideoFrame deep_copy_frame(const QVideoFrame& src){
    if (!src.isValid()){
        return {};
    }

    QVideoFrame source = src;
    if (!source.map(QVideoFrame::ReadOnly)){
        return {};
    }

    QVideoFrame dst(src.surfaceFormat());
    if (!dst.map(QVideoFrame::WriteOnly)){
        source.unmap();
        return {};
    }

    for (int plane = 0; plane < source.planeCount(); plane++){
        const uchar* src_bits = source.bits(plane);
        uchar* dst_bits = dst.bits(plane);
        if (src_bits == nullptr || dst_bits == nullptr){
            continue;
        }

        int src_stride = source.bytesPerLine(plane);
        int dst_stride = dst.bytesPerLine(plane);
        int src_bytes = source.mappedBytes(plane);
        int dst_bytes = dst.mappedBytes(plane);

        if (src_stride == dst_stride){
            std::memcpy(dst_bits, src_bits, std::min(src_bytes, dst_bytes));
            continue;
        }

        //  Both strides are at least the visible row length.
        if (src_stride <= 0 || dst_stride <= 0){
            continue;
        }
        int rows = std::min(src_bytes / src_stride, dst_bytes / dst_stride);
        int copy_bytes = std::min(src_stride, dst_stride);
        for (int r = 0; r < rows; r++){
            std::memcpy(dst_bits + r * dst_stride, src_bits + r * src_stride, copy_bytes);
        }
    }

    source.unmap();
    dst.unmap();

    //  QVideoFrameCache::push_frame() dedups on startTime.
    dst.setStartTime(src.startTime());
    dst.setEndTime(src.endTime());

    return dst;
}



static const char* color_space_name(QVideoFrameFormat::ColorSpace cs){
    switch (cs){
    case QVideoFrameFormat::ColorSpace_BT601:       return "BT601";
    case QVideoFrameFormat::ColorSpace_BT709:       return "BT709";
    case QVideoFrameFormat::ColorSpace_AdobeRgb:    return "AdobeRgb";
    case QVideoFrameFormat::ColorSpace_BT2020:      return "BT2020";
    default:                                        return "Undefined";
    }
}
static const char* color_range_name(QVideoFrameFormat::ColorRange cr){
    switch (cr){
    case QVideoFrameFormat::ColorRange_Video:  return "Limited";
    case QVideoFrameFormat::ColorRange_Full:   return "Full";
    default:                                   return "Unknown";
    }
}


void report_video_path_stats(
    Logger& logger, const char* backend,
    const QVideoFrame& src, bool copy_ok,
    WallClock copy_start, WallClock copy_end
){
    if (!GlobalSettings::instance().VIDEO_PIPELINE->LOG_VIDEO_PATH_STATS){
        return;
    }

    static WallClock last_report = current_time();
    static uint64_t frame_count = 0;
    static uint64_t copy_failures = 0;
    static uint64_t total_micros = 0;

    frame_count++;
    if (!copy_ok){
        copy_failures++;
    }
    total_micros += (uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(
        copy_end - copy_start
    ).count();

    if (copy_end - last_report < std::chrono::seconds(1)){
        return;
    }

    bool gpu = src.handleType() == QVideoFrame::RhiTextureHandle;
    logger.log(
        std::string("VIDEO PATH [") + backend + "]: handle = " +
        (gpu ? "RhiTextureHandle (GPU)" : "NoHandle (CPU)") +
        ", format = " + QVideoFrameFormat::pixelFormatToString(src.pixelFormat()).toStdString() +
        ", size = " + std::to_string(src.width()) + "x" + std::to_string(src.height()) +
        ", planes = " + std::to_string(src.planeCount()) +
        ", colorspace = " + color_space_name(src.surfaceFormat().colorSpace()) +
        ", range = " + color_range_name(src.surfaceFormat().colorRange()) +
        ", fps = " + std::to_string(frame_count) +
        ", copy_failures = " + std::to_string(copy_failures) +
        ", deep_copy avg = " + std::to_string(total_micros / frame_count) + " us",
        copy_failures > 0 ? COLOR_RED : COLOR_PURPLE
    );
    last_report = copy_end;
    frame_count = 0;
    copy_failures = 0;
    total_micros = 0;
}

}
