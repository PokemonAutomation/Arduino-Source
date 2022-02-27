/*  Image Opener
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageStats_H
#define PokemonAutomation_CommonFramework_ImageStats_H

#include <QImage>
#include "Common/Cpp/Exceptions.h"

namespace PokemonAutomation{


inline QImage open_image(const QString& path){
    QImage image(path);
    if (!image.isNull()){
        return image;
    }
    throw FileException(nullptr, __PRETTY_FUNCTION__, "Unable to open image.", path.toStdString());
}


}
#endif
