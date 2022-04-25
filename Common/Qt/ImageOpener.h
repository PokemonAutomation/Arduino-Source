/*  Image Opener
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageOpener_H
#define PokemonAutomation_CommonFramework_ImageOpener_H

#include <QImage>
#include "Common/Cpp/Exceptions.h"

namespace PokemonAutomation{


inline QImage open_image(const QString& path){
    QImage image(path);
    if (!image.isNull()){
        return image;
    }
    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32){
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unable to open image.", path.toStdString());
}


}
#endif
