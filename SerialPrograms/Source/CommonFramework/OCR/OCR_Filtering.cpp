/*  Image Filtering for OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "OCR_Filtering.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace OCR{



BrightnessHistogram::BrightnessHistogram(){
    memset(m_count, 0, sizeof(m_count));
}
BrightnessHistogram::BrightnessHistogram(const ConstImageRef& image)
    : BrightnessHistogram()
{
    for (size_t y = 0; y < image.height(); y++){
        for (size_t x = 0; x < image.width(); x++){
            *this += image.pixel(x, y);
        }
    }
}

void BrightnessHistogram::operator+=(QRgb pixel){
    m_pixels++;
    uint16_t sum = (uint16_t)(qRed(pixel) + qBlue(pixel) + qGreen(pixel));
    m_count[sum / BUCKET_SIZE]++;
}


std::string BrightnessHistogram::dump() const{
    std::string str = "[" + std::to_string(m_count[0]);
    for (size_t c = 1; c < BUCKETS; c++){
        str += ", " + std::to_string(m_count[c]);
    }
    str += "]";
    return str;
}



void TextImageFilter::apply(const ImageRef& image) const{
    if (black_text){
        for (size_t y = 0; y < image.height(); y++){
            for (size_t x = 0; x < image.width(); x++){
                QRgb pixel = image.pixel(x, y);
                int sum = qRed(pixel) + qGreen(pixel) + qBlue(pixel);
                pixel = sum < threshold ? qRgb(0, 0, 0) : qRgb(255, 255, 255);
                image.pixel(x, y) = pixel;

            }
        }
    }else{
        for (size_t y = 0; y < image.height(); y++){
            for (size_t x = 0; x < image.width(); x++){
                QRgb pixel = image.pixel(x, y);
                int sum = qRed(pixel) + qGreen(pixel) + qBlue(pixel);
                pixel = sum > threshold ? qRgb(0, 0, 0) : qRgb(255, 255, 255);
                image.pixel(x, y) = pixel;

            }
        }
    }
}




TextImageFilter make_OCR_filter(const ConstImageRef& image){
    const size_t BUCKETS = BrightnessHistogram::BUCKETS;
    const uint16_t BUCKET_SIZE = BrightnessHistogram::BUCKET_SIZE;

    BrightnessHistogram histogram(image);
//    cout << histogram.dump() << endl;

    uint32_t largest = 0;
    size_t largest_index = 0;
    for (size_t c = 0; c < BUCKETS; c++){
        if (largest < histogram[c]){
            largest = histogram[c];
            largest_index = c;
        }
    }

    size_t lo = std::max((int)largest_index - 2, 0);
    size_t hi = std::min(largest_index + 2, BUCKETS);

#if 0
    size_t below = 0;
    for (size_t c = 0; c < lo; c++){
        below += histogram[c];
    }

    size_t above = 0;
    for (size_t c = hi; c < BUCKETS; c++){
        above += histogram[c];
    }

    TextImageFilter filter;
    if (below < above){
        filter.black_text = false;
        filter.threshold = hi * BUCKET_SIZE;
        filter.threshold = 768 - (768 - filter.threshold) / 2;
    }else{
        filter.black_text = true;
        filter.threshold = lo * BUCKET_SIZE;
        filter.threshold /= 2;
    }

#if 0
    cout << "lo            = " << lo << endl;
    cout << "largest_index = " << largest_index << endl;
    cout << "hi            = " << hi << endl;
    cout << "below = " << below << endl;
    cout << "above = " << above << endl;
    cout << "threshold = " << filter.threshold << endl;
#endif
#else

    TextImageFilter filter;
    if (largest_index < BUCKETS / 3){
        filter.black_text = false;
        filter.threshold = (uint16_t)hi * BUCKET_SIZE;
        filter.threshold = 768 - (768 - filter.threshold) / 2;
    }else{
        filter.black_text = true;
        filter.threshold = (uint16_t)lo * BUCKET_SIZE;
        filter.threshold /= 2;
    }

#if 0
    cout << "lo            = " << lo << endl;
    cout << "largest_index = " << largest_index << endl;
    cout << "hi            = " << hi << endl;
    cout << "threshold = " << filter.threshold << endl;
#endif
#endif


    return filter;
}



void filter_smart(const ImageRef& image){
    OCR::TextImageFilter filter = OCR::make_OCR_filter(image);
    filter.apply(image);
}



void binary_filter_black_text(const ImageRef& image, int max_rgb_sum){
    size_t w = image.width();
    size_t h = image.height();
#if 0
    int min = 0;
    for (pxint_t r = 0; r < h; r++){
        for (pxint_t c = 0; c < w; c++){
            QRgb pixel = image.pixel(c, r);
            int sum = qRed(pixel) + qGreen(pixel) + qBlue(pixel);
            min = std::min(min, sum);
        }
    }
#endif

//    cout << "min = " << min << endl;
//    int threshold = min + 250;
//    int threshold = 250;

    for (size_t r = 0; r < h; r++){
        for (size_t c = 0; c < w; c++){
            QRgb pixel = image.pixel(c, r);
            int sum = qRed(pixel) + qGreen(pixel) + qBlue(pixel);
            pixel = sum < max_rgb_sum ? qRgb(0, 0, 0) : qRgb(255, 255, 255);
            image.pixel(c, r) = pixel;
        }
    }

//    static size_t c = 0;
//    image.save("test-" + QString::number(c++) + ".png");
}

void binary_filter_solid_background(const ImageRef& image, double euclidean_distance){
    euclidean_distance *= euclidean_distance;
    ImageStats stats = image_border_stats(image);
    size_t w = image.width();
    size_t h = image.height();
    for (size_t r = 0; r < h; r++){
        for (size_t c = 0; c < w; c++){
            QRgb pixel = image.pixel(c, r);
            double R = (double)qRed(pixel) - stats.average.r;
            double g = (double)qGreen(pixel) - stats.average.g;
            double b = (double)qBlue(pixel) - stats.average.b;
            bool text = R*R + g*g + b*b >= euclidean_distance;
            image.pixel(c, r) = text ? 0 : 0x00ffffff;
        }
    }
}





}
}

