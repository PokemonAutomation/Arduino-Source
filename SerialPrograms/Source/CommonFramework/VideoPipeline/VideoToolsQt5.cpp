/*  Video Tools (QT5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5

#include <sstream>
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "VideoToolsQt5.h"

namespace PokemonAutomation{



QImage frame_to_image(Logger& logger, QVideoFrame frame, bool flip_vertical){
    if (!frame.isValid()){
        return QImage();
    }
    if (!frame.map(QAbstractVideoBuffer::ReadOnly)){
        logger.log("Unable to map QVideoFrame.");
        return QImage();
    }

    try{
        if (frame.pixelFormat() == QVideoFrame::Format_Jpeg){
            QImage image;
            image.loadFromData(frame.bits(0), frame.mappedBytes(), "JPG");
            frame.unmap();
            if (flip_vertical){
                image = image.mirrored(false, flip_vertical);
            }
            return image;
        }

        QImage::Format format = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
        QImage image(
            frame.bits(),
            frame.width(),
            frame.height(),
            frame.bytesPerLine(),
            format
        );
        int64_t key = image.cacheKey();
        image = image.convertToFormat(QImage::Format_RGB32);
        frame.unmap();
        image = image.mirrored(false, flip_vertical);
        if (key == image.cacheKey()){
            image = image.copy();
        }
        return image;
    }catch (...){
        frame.unmap();
    }
    return QImage();
}


bool determine_frame_orientation(
    Logger& logger, const QImage& reference, const QImage& image,
    bool& flip_vertical
){
    std::stringstream ss;
    ss << "Attempting to determine frame orientation... ";

    if (reference.isNull()){
        ss << "Image is null.";
        logger.log(ss.str(), COLOR_RED);
        return false;
    }
    if (image.isNull()){
        ss << "Frame is null.";
        logger.log(ss.str(), COLOR_RED);
        return false;
    }

    ImageStats ref_stats = image_stats(reference);
    double ref_stddev = ref_stats.stddev.sum();
    if (ref_stddev < 10){
        std::stringstream ss;
        ss << "Image has too little detail. (rmsd = " << ref_stddev << ")";
        logger.log(ss.str(), COLOR_RED);
        return false;
    }
    ImageStats img_stats = image_stats(image);
    double img_stddev = img_stats.stddev.sum();
    if (img_stddev < 10){
        std::stringstream ss;
        ss << "Frame has too little detail. (rmsd = " << img_stddev << ")";
        logger.log(ss.str(), COLOR_RED);
        return false;
    }

    double stddev_inv = (ref_stddev + img_stddev) * 0.5;
    ss << "\n";
    ss << "    Stddev = " << stddev_inv << "\n";

    stddev_inv = 1 / stddev_inv;

    double identity         = stddev_inv * ImageMatch::pixel_RMSD(reference, image);
    double flipped_vertical = stddev_inv * ImageMatch::pixel_RMSD(reference, image.mirrored(false, true));
    ss << "    Identity = " << identity << "\n";
    ss << "    Flipped Vertical = " << flipped_vertical << "\n";

    ss << "    Orientation: ";
    bool ok = false;
    do{
        if (identity < 0.01 && flipped_vertical > 0.1){
            flip_vertical = false;
            ok = true;
            ss << "Identity";
            break;
        }
        if (flipped_vertical < 0.01 && identity > 0.1){
            flip_vertical = true;
            ok = true;
            ss << "Flipped Vertical";
            break;
        }
        ss << "Unknown";
    }while (false);

    logger.log(ss.str(), ok ? COLOR_BLUE : COLOR_RED);

    return ok;
}











}
#endif
