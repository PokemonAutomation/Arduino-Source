/*  Image Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "CommonFramework/Tools/VideoFeed.h"
#include "ImageTools.h"

namespace PokemonAutomation{



QImage image_diff_greyscale(const QImage& x, const QImage& y){
    if (x.isNull() || y.isNull()){
        return QImage();
    }
    if (x.width() != y.width()){
        return QImage();
    }
    if (x.height() != y.height()){
        return QImage();
    }

    QImage image(x.width(), x.height(), x.format());
    pxint_t width = x.width();
    pxint_t height = x.height();
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
            QRgb px = x.pixel(c, r);
            QRgb py = y.pixel(c, r);
            if (qAlpha(px) == 0 || qAlpha(py) == 0){
                image.setPixel(c, r, qRgb(0, 0, 0));
            }else{
                double distance = euclidean_distance(px, py);
                distance *= 0.57735026918962576451;  //  1 / sqrt(3)
                int dist_int = std::min((int)distance, 255);
                image.setPixel(c, r, qRgb(dist_int, dist_int, dist_int));
            }
        }
    }
    return image;
}












}

