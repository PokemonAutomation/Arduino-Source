/*  Image Filtering for OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_Filtering_H
#define PokemonAutomation_OCR_Filtering_H

#include <QImage>
#include "CommonFramework/ImageTypes/ImageReference.h"

namespace PokemonAutomation{
namespace OCR{


class BrightnessHistogram{
public:
    static const size_t BUCKETS = 24;
    static const uint16_t BUCKET_SIZE = 3 * 256 / BUCKETS;

public:
    BrightnessHistogram();
    BrightnessHistogram(const ConstImageRef& image);

    void operator+=(QRgb pixel);

    uint32_t operator[](size_t bucket) const{
        return m_count[bucket];
    }
    const uint32_t* histogram() const{
        return m_count;
    }

    std::string dump() const;


private:
    size_t m_pixels = 0;
    uint32_t m_count[BUCKETS];
};



struct TextImageFilter{
    bool black_text;
    uint16_t threshold;

    void apply(const ImageRef& image) const;
};


TextImageFilter make_OCR_filter(const ConstImageRef& image);


void filter_smart(const ImageRef& image);

void binary_filter_black_text(const ImageRef& image, int max_rgb_sum = 250);
void binary_filter_solid_background(const ImageRef& image, double euclidean_distance = 128);




}
}
#endif
