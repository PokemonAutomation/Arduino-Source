/*  Video Tools (QT5)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#if QT_VERSION_MAJOR == 5

#include "VideoToolsQt5.h"

namespace PokemonAutomation{



QImage frame_to_image(Logger& logger, QVideoFrame frame, bool flip_vertical){
    if (frame.isValid()){
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



}
#endif
