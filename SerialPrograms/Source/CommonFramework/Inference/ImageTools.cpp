/*  Image Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "CommonFramework/Tools/VideoFeed.h"
#include "ImageTools.h"

namespace PokemonAutomation{



QString FloatPixel::to_string() const{
    return "{" + QString::number(r) + ", " + QString::number(g) + ", " + QString::number(b) + "}";
}
double FloatPixel::stddev() const{
    double mean = (r + g + b) / 3;
    double rd = (r - mean);
    double gd = (g - mean);
    double bd = (b - mean);
    return std::sqrt((rd*rd + gd*gd + bd*bd) / 2);
}

FloatPixel abs(const FloatPixel& x){
    return FloatPixel{
        x.r < 0 ? -x.r : x.r,
        x.g < 0 ? -x.g : x.g,
        x.b < 0 ? -x.b : x.b,
    };
}
double euclidean_distance(const FloatPixel& x, const FloatPixel& y){
    FloatPixel p = x - y;
    p *= p;
    return std::sqrt(p.r + p.g + p.b);
}



QImage extract_box(const QImage& image, const InferenceBox& box){
    return image.copy(
        (int)(image.width() * box.x + 0.5),
        (int)(image.height() * box.y + 0.5),
        (int)(image.width() * box.width + 0.5),
        (int)(image.height() * box.height + 0.5)
    );
}
FloatPixel pixel_average(const QImage& image){
    int w = image.width();
    int h = image.height();
    FloatPixel sum;
    for (int r = 0; r < h; r++){
        for (int c = 0; c < w; c++){
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
    int w = image.width();
    int h = image.height();
    if (w * h <= 1){
        return FloatPixel();
    }
    FloatPixel sum;
    FloatPixel sqr_sum;
    for (int r = 0; r < h; r++){
        for (int c = 0; c < w; c++){
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

ImageStats pixel_stats(const QImage& image){
    int w = image.width();
    int h = image.height();
    if (w * h <= 1){
        return ImageStats();
    }
    FloatPixel sum;
    FloatPixel sqr_sum;
    for (int r = 0; r < h; r++){
        for (int c = 0; c < w; c++){
            FloatPixel p(image.pixel(c, r));
            sum += p;
            sqr_sum += p * p;
        }
    }
    size_t total = w * h;
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

