/*  Video Frame Conversion
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  YUV video frames (NV12, NV21, YUV420P, YV12, YUYV, UYVY) to pictures on
 *  the CPU, without QVideoFrame::toImage().
 *
 *  toImage() converts on the GPU where it can (a QRhi for each thread that
 *  calls it). On some computers the process memory then grows with each
 *  conversion and is only given back by restarting PA: 10-13 KB a
 *  conversion, 0.3-1.6 GB an hour while a program with screen watchers
 *  runs (Windows, Intel UHD 630, a 1280 x 720 NV12 capture card, Qt
 *  6.10.2). The same frames converted here do not grow it.
 *
 *  VideoFrameConverter compares its own conversion with toImage() on the
 *  first frames with colors and takes over only when the colors match.
 *  The environment variable PA_QT_FRAME_CONVERSION=1 keeps toImage().
 *
 */

#ifndef PokemonAutomation_VideoPipeline_VideoFrameConversion_H
#define PokemonAutomation_VideoPipeline_VideoFrameConversion_H

#include <atomic>
#include <mutex>
#include <QVideoFrame>
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"

namespace PokemonAutomation{

class Logger;


//  YUV to RGB: the factors times 1024.
struct YuvConversion{
    const char* name;
    int y;
    int rv;
    int gu;
    int gv;
    int bu;
    int y_offset;
    //  The colors between their samples (as the GPU samples them), else the
    //  nearest sample (as Qt's CPU conversion does).
    bool bilinear;
};

//  BT.601 / BT.709, limited / full range, nearest / bilinear colors.
constexpr size_t YUV_CONVERSION_COUNT = 8;
extern const YuvConversion YUV_CONVERSIONS[YUV_CONVERSION_COUNT];

//  A mapped YUV frame as an ARGB32 picture. Empty for other formats.
ImageRGB32 yuv_frame_to_image(const QVideoFrame& mapped, const YuvConversion& conversion);
//  The same, mapping the frame for it (empty if it cannot be mapped).
ImageRGB32 yuv_frame_to_image_mapping(QVideoFrame frame, const YuvConversion& conversion);

//  QVideoFrame::toImage() as an ARGB32 picture.
ImageRGB32 qt_frame_to_image(const QVideoFrame& frame);

//  The mean and the largest difference of the color channels. False if the
//  sizes differ.
bool compare_images(const ImageViewRGB32& a, const ImageViewRGB32& b, double& mean, int& largest);

//  Enough colors to tell the conversions apart (not a black, white or gray
//  screen).
bool image_has_colors(const ImageViewRGB32& image);


class VideoFrameConverter{
public:
    VideoFrameConverter(Logger& logger);

    //  The frame as an ARGB32 picture: the own conversion once it matched
    //  toImage(), else toImage().
    ImageRGB32 convert(const QVideoFrame& frame);

private:
    void compare(const QVideoFrame& frame, const ImageRGB32& qt);

private:
    static constexpr int DECIDING = -1;
    static constexpr int USE_QT = -2;
    static constexpr int FRAMES_TO_COMPARE = 8;

    Logger& m_logger;
    std::atomic<int> m_choice;

    std::mutex m_lock;
    WallClock m_last_compare = WallClock::min();
    int m_compared = 0;
    int m_tries = 0;
    double m_mean[YUV_CONVERSION_COUNT];
    int m_largest[YUV_CONVERSION_COUNT];
    //  The frames the choice is for.
    int m_width = 0;
    int m_height = 0;
    int m_format = 0;
};



}
#endif
