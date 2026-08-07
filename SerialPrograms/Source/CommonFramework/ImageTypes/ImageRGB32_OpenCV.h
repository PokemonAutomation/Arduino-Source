/*  Image (RGB32) Qt
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageRGB32_OpenCV_H
#define PokemonAutomation_CommonFramework_ImageRGB32_OpenCV_H

#include <opencv2/core/mat.hpp>
#include "ImageRGB32.h"

namespace PokemonAutomation{


ImageRGB32 OpenCV_scale_image(const ImageViewRGB32& image, size_t width, size_t height);

ImageRGB32 OpenCV_load_image(const std::string& filename);
bool OpenCV_save_image(const ImageViewRGB32& image, const std::string& filename);



class ImageRGB32OpenCV : public CustomImageRGB32Owner{
public:
    ImageRGB32OpenCV(cv::Mat image);

    virtual ImageViewRGB32 get_view() const override;


private:
    cv::Mat m_image;
};



}
#endif
