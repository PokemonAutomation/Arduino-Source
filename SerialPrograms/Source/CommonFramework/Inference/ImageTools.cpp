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


InferenceBox translate_to_parent(
    const QImage& original_image,
    const InferenceBox& inference_box,
    const PixelBox& box
){
    double width = original_image.width();
    double height = original_image.height();
    int box_x = (int)(width * inference_box.x + 0.5);
    int box_y = (int)(height * inference_box.y + 0.5);
    return InferenceBox(
        (box_x + box.min_x) / width,
        (box_y + box.min_y) / height,
        (box.max_x - box.min_x) / width,
        (box.max_y - box.min_y) / height
    );
}

QImage extract_box(const QImage& image, const PixelBox& box){
    return image.copy(box.min_x, box.min_y, box.width(), box.height());
}
QImage extract_box(const QImage& image, const InferenceBox& box){
    return image.copy(
        (int)(image.width() * box.x + 0.5),
        (int)(image.height() * box.y + 0.5),
        (int)(image.width() * box.width + 0.5),
        (int)(image.height() * box.height + 0.5)
    );
}

double image_diff(const QImage& x, const QImage& y){
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

    int width = x.width();
    int height = x.height();
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
            sum += euclidean_distance(x.pixel(c, r), y.pixel(c, r));
        }
    }

    return std::sqrt(sum / ((size_t)width * height));
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
ImageStats object_stats(const QImage& image, const FillMatrix& matrix, const FillGeometry& object){
    int w = image.width();
    int h = image.height();
    if (w * h <= 1){
        return ImageStats();
    }
    FloatPixel sum;
    FloatPixel sqr_sum;
    for (int r = 0; r < h; r++){
        for (int c = 0; c < w; c++){
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


bool is_black(const QImage& image, double max_rgb_sum, double max_stddev_sum){
    ImageStats stats = pixel_stats(image);
    double average = stats.average.sum();
    double stddev = stats.stddev.sum();
    return average < 100 && stddev < 10;
}




}

