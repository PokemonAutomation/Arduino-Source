/*  Image (RGB32) Qt
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageRGB32Qt_H
#define PokemonAutomation_CommonFramework_ImageRGB32Qt_H

#include <iostream>
#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "ImageRGB32.h"

namespace PokemonAutomation{


//  Return a shallow copy-on-write reference that points to this buffer. (fast)
QImage to_QImage_ref(const ImageViewRGB32& image);

//  Return a copy that owns its own buffer. (slow)
QImage to_QImage_owning(const ImageViewRGB32& image);

QImage scaled_to_QImage(const ImageViewRGB32& image, size_t width, size_t height);

ImageRGB32 QImage_to_ImageRGB32(QImage image);




class ImageRGB32Qt : public CustomImageRGB32Owner{
public:
    ImageRGB32Qt(QImage image);

    virtual ImageViewRGB32 get_view() const override;


private:
    QImage m_image;
};





}
#endif
