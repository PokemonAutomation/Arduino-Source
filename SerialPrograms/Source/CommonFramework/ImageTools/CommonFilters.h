/*  Common Filters
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Return a set of connected clusters.
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageTools_CommonFilters_H
#define PokemonAutomation_CommonFramework_ImageTools_CommonFilters_H

#include "CellMatrix.h"

namespace PokemonAutomation{


struct WhiteFilter{
    WhiteFilter(int p_min_value)
        : min_value(p_min_value)
    {}

    int min_value;
    size_t count = 0;

    void operator()(CellMatrix::ObjectID& cell, const QImage& image, int x, int y){
        QRgb pixel = image.pixel(x, y);
        int set = qRed(pixel) >= min_value && qGreen(pixel) >= min_value && qBlue(pixel) >= min_value;
//        int set = (pixel & 0x00c0c080) == 0x00c0c080 ? 1 : 0;
        cell = set;
        count += set;
    }
};


struct BlackFilter{
    BlackFilter(int p_max_value)
        : min_value(p_max_value)
    {}

    int min_value;
    size_t count = 0;

    void operator()(CellMatrix::ObjectID& cell, const QImage& image, int x, int y){
        QRgb pixel = image.pixel(x, y);
        int set = qRed(pixel) <= min_value && qGreen(pixel) <= min_value && qBlue(pixel) <= min_value;
//        int set = (pixel & 0x00c0c080) == 0x00c0c080 ? 1 : 0;
        cell = set;
        count += set;
    }
};


struct BrightFilter{
    BrightFilter(int p_min_rgb_sum)
        : min_value(p_min_rgb_sum)
    {}

    int min_value;
    size_t count = 0;

    void operator()(CellMatrix::ObjectID& cell, const QImage& image, int x, int y){
        QRgb pixel = image.pixel(x, y);
        int set = qRed(pixel) + qGreen(pixel) + qBlue(pixel) >= min_value;
//        int set = (pixel & 0x00c0c080) == 0x00c0c080 ? 1 : 0;
        cell = set;
        count += set;
    }
};



}
#endif
