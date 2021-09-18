/*  Image Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "CommonFramework/Tools/VideoFeed.h"
#include "ImageTools.h"

namespace PokemonAutomation{


ImageFloatBox translate_to_parent(
    const QImage& original_image,
    const ImageFloatBox& inference_box,
    const ImagePixelBox& box
){
    double width = original_image.width();
    double height = original_image.height();
    pxint_t box_x = (pxint_t)(width * inference_box.x + 0.5);
    pxint_t box_y = (pxint_t)(height * inference_box.y + 0.5);
    return ImageFloatBox(
        (box_x + box.min_x) / width,
        (box_y + box.min_y) / height,
        (box.max_x - box.min_x) / width,
        (box.max_y - box.min_y) / height
    );
}


double image_diff_total(const QImage& x, const QImage& y){
    if (x.isNull() || y.isNull()){
        return -1;
    }
    if (x.width() != y.width()){
        return -1;
    }
    if (x.height() != y.height()){
        return -1;
    }

    double sum = 0;

    pxint_t width = x.width();
    pxint_t height = x.height();
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
//            sum += euclidean_distance(x.pixel(c, r), y.pixel(c, r));
            FloatPixel p = FloatPixel(x.pixel(c, r)) - FloatPixel(y.pixel(c, r));
            p *= p;
            sum += p.sum();
        }
    }

//    return std::sqrt(sum / ((size_t)width * height));
    return std::sqrt(sum / ((size_t)width * height));
}
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



FloatPixel pixel_average(const QImage& image){
    pxint_t w = image.width();
    pxint_t h = image.height();
    FloatPixel sum;
    for (pxint_t r = 0; r < h; r++){
        for (pxint_t c = 0; c < w; c++){
            sum += FloatPixel(image.pixel(c, r));
        }
    }
    size_t total = w * h;
    sum.r /= total;
    sum.g /= total;
    sum.b /= total;
    return sum;
}
FloatPixel pixel_average_normalized(const QImage& image){
    FloatPixel average = pixel_average(image);
    double sum = average.sum();
    if (sum == 0){
        return FloatPixel();
    }
    return average / sum;
}


FloatPixel pixel_stddev(const QImage& image){
    pxint_t w = image.width();
    pxint_t h = image.height();
    if (w * h <= 1){
        return FloatPixel();
    }
    FloatPixel sum;
    FloatPixel sqr_sum;
    for (pxint_t r = 0; r < h; r++){
        for (pxint_t c = 0; c < w; c++){
            FloatPixel p(image.pixel(c, r));
            sum += p;
            sqr_sum += p * p;
        }
    }
    size_t total = w * h;
    FloatPixel variance = (sqr_sum - sum*sum / total) / (total - 1);
    return FloatPixel(
        std::sqrt(variance.r),
        std::sqrt(variance.g),
        std::sqrt(variance.b)
    );
}

ImageStats object_stats(const QImage& image, const CellMatrix& matrix, const FillGeometry& object){
    pxint_t w = image.width();
    pxint_t h = image.height();
    if (w * h <= 1){
        return ImageStats();
    }
    FloatPixel sum;
    FloatPixel sqr_sum;
    for (pxint_t r = 0; r < h; r++){
        for (pxint_t c = 0; c < w; c++){
            if (matrix[r][c] == object.id){
                FloatPixel p(image.pixel(c, r));
                sum += p;
                sqr_sum += p * p;
            }
        }
    }
    size_t total = object.area;
    FloatPixel variance = (sqr_sum - sum*sum / total) / (total - 1);
    return ImageStats{
        sum / total,
        FloatPixel(
            std::sqrt(variance.r),
            std::sqrt(variance.g),
            std::sqrt(variance.b)
        )
    };
}











}

