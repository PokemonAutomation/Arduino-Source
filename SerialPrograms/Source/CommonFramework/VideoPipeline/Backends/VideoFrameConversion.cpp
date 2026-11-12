/*  Video Frame Conversion
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <vector>
#include <QImage>
#include <QVideoFrameFormat>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/ImageTypes/ImageRGB32_Qt.h"
#include "VideoFrameConversion.h"

namespace PokemonAutomation{



const YuvConversion YUV_CONVERSIONS[YUV_CONVERSION_COUNT]{
    {"BT.601 limited, nearest",  1192, 1634, 401, 833, 2065, 16, false},
    {"BT.709 limited, nearest",  1192, 1836, 218, 546, 2163, 16, false},
    {"BT.601 full, nearest",     1024, 1436, 352, 731, 1815, 0,  false},
    {"BT.709 full, nearest",     1024, 1613, 192, 479, 1900, 0,  false},
    {"BT.601 limited, bilinear", 1192, 1634, 401, 833, 2065, 16, true},
    {"BT.709 limited, bilinear", 1192, 1836, 218, 546, 2163, 16, true},
    {"BT.601 full, bilinear",    1024, 1436, 352, 731, 1815, 0,  true},
    {"BT.709 full, bilinear",    1024, 1613, 192, 479, 1900, 0,  true},
};


namespace{

inline uint32_t to_byte(int scaled){
    return (uint32_t)(std::clamp(scaled + 512, 0, 255 * 1024 + 1023) >> 10);
}
inline uint32_t yuv_pixel(const YuvConversion& m, int y, int u, int v){
    const int yy = (y - m.y_offset) * m.y;
    u -= 128;
    v -= 128;
    const uint32_t r = to_byte(yy + m.rv * v);
    const uint32_t g = to_byte(yy - m.gu * u - m.gv * v);
    const uint32_t b = to_byte(yy + m.bu * u);
    return 0xff000000u | (r << 16) | (g << 8) | b;
}

//  The two color samples for a pixel, and their weights in quarters. A
//  color sample covers 2 pixels; the GPU samples it at the pixel's center
//  (index / 2 - 0.25): between the sample before and this one, or this one
//  and the one after.
struct ChromaTap{
    int i0;
    int i1;
    int w0;
    int w1;
};
inline ChromaTap chroma_tap(int pixel, int samples, bool subsampled, bool bilinear){
    if (!subsampled){
        return {pixel, pixel, 4, 0};
    }
    const int k = pixel / 2;
    if (!bilinear){
        return {k, k, 4, 0};
    }
    if ((pixel & 1) == 0){
        return {std::max(k - 1, 0), k, 1, 3};
    }
    return {k, std::min(k + 1, samples - 1), 3, 1};
}

//  Y(row, column), and the color samples: U and V of a sample row, as
//  pointers with a step.
struct YuvLayout{
    const uchar* y = nullptr;
    int y_stride = 0;
    int y_step = 1;
    const uchar* u = nullptr;
    const uchar* v = nullptr;
    int uv_stride = 0;
    int uv_step = 1;
    int uv_width = 0;
    int uv_height = 0;
    bool vertical_subsampled = true;
};

bool yuv_layout(const QVideoFrame& frame, YuvLayout& layout){
    const int width = frame.width();
    const int height = frame.height();
    const uchar* plane0 = frame.bits(0);
    const int stride0 = frame.bytesPerLine(0);
    if (width <= 0 || height <= 0 || plane0 == nullptr || stride0 <= 0){
        return false;
    }
    layout.uv_width = (width + 1) / 2;
    layout.uv_height = (height + 1) / 2;
    switch (frame.pixelFormat()){
    case QVideoFrameFormat::Format_NV12:
    case QVideoFrameFormat::Format_NV21:{
        const uchar* plane1 = frame.planeCount() >= 2 ? frame.bits(1) : nullptr;
        int stride1 = frame.planeCount() >= 2 ? frame.bytesPerLine(1) : stride0;
        if (plane1 == nullptr){
            plane1 = plane0 + (size_t)stride0 * (size_t)height;
            stride1 = stride0;
        }
        const bool nv12 = frame.pixelFormat() == QVideoFrameFormat::Format_NV12;
        layout.y = plane0;
        layout.y_stride = stride0;
        layout.u = plane1 + (nv12 ? 0 : 1);
        layout.v = plane1 + (nv12 ? 1 : 0);
        layout.uv_stride = stride1;
        layout.uv_step = 2;
        return true;
    }
    case QVideoFrameFormat::Format_YUV420P:
    case QVideoFrameFormat::Format_YV12:{
        if (frame.planeCount() < 3 || frame.bits(1) == nullptr || frame.bits(2) == nullptr ||
            frame.bytesPerLine(1) != frame.bytesPerLine(2)
        ){
            return false;
        }
        const bool yv12 = frame.pixelFormat() == QVideoFrameFormat::Format_YV12;
        layout.y = plane0;
        layout.y_stride = stride0;
        layout.u = frame.bits(yv12 ? 2 : 1);
        layout.v = frame.bits(yv12 ? 1 : 2);
        layout.uv_stride = frame.bytesPerLine(1);
        layout.uv_step = 1;
        return true;
    }
    case QVideoFrameFormat::Format_YUYV:
    case QVideoFrameFormat::Format_UYVY:{
        //  YUYV: Y0 U Y1 V. UYVY: U Y0 V Y1.
        const bool yuyv = frame.pixelFormat() == QVideoFrameFormat::Format_YUYV;
        layout.y = plane0 + (yuyv ? 0 : 1);
        layout.y_stride = stride0;
        layout.y_step = 2;
        layout.u = plane0 + (yuyv ? 1 : 0);
        layout.v = plane0 + (yuyv ? 3 : 2);
        layout.uv_stride = stride0;
        layout.uv_step = 4;
        layout.uv_height = height;
        layout.vertical_subsampled = false;
        return true;
    }
    default:
        return false;
    }
}

}



ImageRGB32 yuv_frame_to_image(const QVideoFrame& mapped, const YuvConversion& conversion){
    YuvLayout layout;
    if (!yuv_layout(mapped, layout)){
        return ImageRGB32();
    }
    const int width = mapped.width();
    const int height = mapped.height();

    std::vector<ChromaTap> columns((size_t)width);
    for (int c = 0; c < width; c++){
        columns[(size_t)c] = chroma_tap(c, layout.uv_width, true, conversion.bilinear);
    }
    //  The color samples of the row, mixed from the two sample rows (times
    //  4).
    std::vector<int> u_line((size_t)layout.uv_width);
    std::vector<int> v_line((size_t)layout.uv_width);

    ImageRGB32 image((size_t)width, (size_t)height);
    for (int row = 0; row < height; row++){
        const ChromaTap rows = chroma_tap(row, layout.uv_height, layout.vertical_subsampled, conversion.bilinear);
        const uchar* u0 = layout.u + (size_t)rows.i0 * layout.uv_stride;
        const uchar* v0 = layout.v + (size_t)rows.i0 * layout.uv_stride;
        const uchar* u1 = layout.u + (size_t)rows.i1 * layout.uv_stride;
        const uchar* v1 = layout.v + (size_t)rows.i1 * layout.uv_stride;
        for (int s = 0; s < layout.uv_width; s++){
            const size_t at = (size_t)s * layout.uv_step;
            u_line[(size_t)s] = rows.w0 * u0[at] + rows.w1 * u1[at];
            v_line[(size_t)s] = rows.w0 * v0[at] + rows.w1 * v1[at];
        }

        const uchar* y = layout.y + (size_t)row * layout.y_stride;
        uint32_t* out = (uint32_t*)((char*)image.data() + (size_t)row * image.bytes_per_row());
        for (int c = 0; c < width; c++){
            const ChromaTap& t = columns[(size_t)c];
            const int u = (t.w0 * u_line[(size_t)t.i0] + t.w1 * u_line[(size_t)t.i1] + 8) >> 4;
            const int v = (t.w0 * v_line[(size_t)t.i0] + t.w1 * v_line[(size_t)t.i1] + 8) >> 4;
            out[c] = yuv_pixel(conversion, y[(size_t)c * layout.y_step], u, v);
        }
    }
    return image;
}
ImageRGB32 yuv_frame_to_image_mapping(QVideoFrame frame, const YuvConversion& conversion){
    if (!frame.map(QVideoFrame::ReadOnly)){
        return ImageRGB32();
    }
    ImageRGB32 image = yuv_frame_to_image(frame, conversion);
    frame.unmap();
    return image;
}

ImageRGB32 qt_frame_to_image(const QVideoFrame& frame){
    QImage image = frame.toImage();
    QImage::Format format = image.format();
    if (format != QImage::Format_ARGB32 && format != QImage::Format_RGB32){
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    return QImage_to_ImageRGB32(std::move(image));
}


bool compare_images(const ImageViewRGB32& a, const ImageViewRGB32& b, double& mean, int& largest){
    if (!a || !b || a.width() != b.width() || a.height() != b.height()){
        return false;
    }
    uint64_t sum = 0;
    largest = 0;
    for (size_t row = 0; row < a.height(); row++){
        for (size_t c = 0; c < a.width(); c++){
            const uint32_t pa = a.pixel(c, row);
            const uint32_t pb = b.pixel(c, row);
            for (int shift = 0; shift < 24; shift += 8){
                const int d = std::abs((int)((pa >> shift) & 0xff) - (int)((pb >> shift) & 0xff));
                sum += (uint64_t)d;
                largest = std::max(largest, d);
            }
        }
    }
    mean = (double)sum / (3.0 * (double)a.width() * (double)a.height());
    return true;
}

bool image_has_colors(const ImageViewRGB32& image){
    if (!image || image.width() < 8 || image.height() < 8){
        return false;
    }
    //  Every 4th pixel: the mean distance of the colors from gray, and how
    //  much the brightness changes.
    uint64_t chroma = 0;
    uint64_t sum = 0;
    uint64_t sum_sq = 0;
    uint64_t count = 0;
    for (size_t row = 0; row < image.height(); row += 4){
        for (size_t c = 0; c < image.width(); c += 4){
            const uint32_t p = image.pixel(c, row);
            const int r = (p >> 16) & 0xff;
            const int g = (p >> 8) & 0xff;
            const int b = p & 0xff;
            chroma += (uint64_t)(std::abs(r - g) + std::abs(b - g));
            const uint64_t luma = (uint64_t)(r + g + b);
            sum += luma;
            sum_sq += luma * luma;
            count++;
        }
    }
    const double mean = (double)sum / count;
    const double variance = (double)sum_sq / count - mean * mean;
    return (double)chroma / count >= 20 && variance >= 30.0 * 30.0;
}



VideoFrameConverter::VideoFrameConverter(Logger& logger)
    : m_logger(logger)
    , m_choice(DECIDING)
{
    for (size_t i = 0; i < YUV_CONVERSION_COUNT; i++){
        m_mean[i] = 0;
        m_largest[i] = 0;
    }
    const char* env = std::getenv("PA_QT_FRAME_CONVERSION");
    if (env != nullptr && env[0] == '1'){
        m_choice.store(USE_QT);
        m_logger.log("Video frames: PA_QT_FRAME_CONVERSION=1, converted by Qt (QVideoFrame::toImage()).");
    }
}

ImageRGB32 VideoFrameConverter::convert(const QVideoFrame& frame){
    const int choice = m_choice.load(std::memory_order_acquire);
    if (choice >= 0){
        if (frame.width() == m_width && frame.height() == m_height && (int)frame.pixelFormat() == m_format){
            ImageRGB32 image = yuv_frame_to_image_mapping(frame, YUV_CONVERSIONS[choice]);
            if (image){
                return image;
            }
        }
        return qt_frame_to_image(frame);
    }

    ImageRGB32 qt = qt_frame_to_image(frame);
    if (choice == DECIDING){
        compare(frame, qt);
    }
    return qt;
}

void VideoFrameConverter::compare(const QVideoFrame& frame, const ImageRGB32& qt){
    std::unique_lock<std::mutex> lg(m_lock, std::try_to_lock);
    if (!lg.owns_lock() || m_choice.load(std::memory_order_acquire) != DECIDING){
        return;
    }
    //  At most one frame a second: a comparison takes 8 conversions.
    const WallClock now = current_time();
    if (now < m_last_compare + std::chrono::seconds(1)){
        return;
    }
    m_last_compare = now;
    if (!image_has_colors(qt)){
        return;
    }

    if (m_compared == 0){
        m_width = frame.width();
        m_height = frame.height();
        m_format = (int)frame.pixelFormat();
    }else if (frame.width() != m_width || frame.height() != m_height || (int)frame.pixelFormat() != m_format){
        //  Another video: start again.
        m_compared = 0;
        m_width = frame.width();
        m_height = frame.height();
        m_format = (int)frame.pixelFormat();
        for (size_t i = 0; i < YUV_CONVERSION_COUNT; i++){
            m_mean[i] = 0;
            m_largest[i] = 0;
        }
    }

    QVideoFrame copy = frame;
    if (!copy.map(QVideoFrame::ReadOnly)){
        if (++m_tries >= 10){
            m_choice.store(USE_QT, std::memory_order_release);
            m_logger.log("Video frames: they cannot be mapped. Converted by Qt (QVideoFrame::toImage()).");
        }
        return;
    }
    bool known = true;
    for (size_t i = 0; i < YUV_CONVERSION_COUNT && known; i++){
        ImageRGB32 own = yuv_frame_to_image(copy, YUV_CONVERSIONS[i]);
        double mean = 0;
        int largest = 0;
        known = compare_images(own, qt, mean, largest);
        m_mean[i] += mean;
        m_largest[i] = std::max(m_largest[i], largest);
    }
    copy.unmap();
    if (!known){
        m_choice.store(USE_QT, std::memory_order_release);
        m_logger.log(
            "Video frames: " + QVideoFrameFormat::pixelFormatToString(frame.pixelFormat()).toStdString() +
            " " + std::to_string(frame.width()) + " x " + std::to_string(frame.height()) +
            ", a format the own conversion does not know. Converted by Qt (QVideoFrame::toImage())."
        );
        return;
    }
    if (++m_compared < FRAMES_TO_COMPARE){
        return;
    }

    size_t best = 0;
    std::string table;
    for (size_t i = 0; i < YUV_CONVERSION_COUNT; i++){
        char item[96];
        snprintf(item, sizeof(item), "%s%s %.2f / %d", i == 0 ? "" : "; ", YUV_CONVERSIONS[i].name, m_mean[i] / m_compared, m_largest[i]);
        table += item;
        if (m_mean[i] < m_mean[best]){
            best = i;
        }
    }
    const double best_mean = m_mean[best] / m_compared;
    const std::string what =
        QVideoFrameFormat::pixelFormatToString(frame.pixelFormat()).toStdString() + " " +
        std::to_string(frame.width()) + " x " + std::to_string(frame.height());
    if (best_mean <= 1.5){
        m_choice.store((int)best, std::memory_order_release);
        m_logger.log(
            "Video frames (" + what + "): converted without Qt from now on (" + YUV_CONVERSIONS[best].name +
            "). Color difference with QVideoFrame::toImage() over " + std::to_string(m_compared) +
            " frames, mean / largest: " + table + "."
        );
    }else{
        m_choice.store(USE_QT, std::memory_order_release);
        m_logger.log(
            "Video frames (" + what + "): the own conversion's colors differ from QVideoFrame::toImage(), "
            "which converts them. Mean / largest difference over " + std::to_string(m_compared) +
            " frames: " + table + ".",
            COLOR_ORANGE
        );
    }
}



}
