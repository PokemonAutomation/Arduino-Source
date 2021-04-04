/*  Shiny Filters
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyFilters_H
#define PokemonAutomation_PokemonSwSh_ShinyFilters_H

#include "CommonFramework/Inference/FillMatrix.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



struct BrightYellowLightFilter{
    size_t count = 0;

    void operator()(FillMatrix::ObjectID& cell, const QImage& image, size_t x, size_t y){
        QRgb pixel = image.pixel(x, y);
//        int set = qRed(pixel) > 160 && qGreen(pixel) > 160 && qBlue(pixel) > 128;
        int set = (pixel & 0x00c0c080) == 0x00c0c080 ? 1 : 0;
        cell = set;
        count += set;
    }
};

struct BrightYellowLightFilterDebug{
    size_t count = 0;

    void operator()(FillMatrix::ObjectID& cell, QImage& image, size_t x, size_t y){
        QRgb pixel = image.pixel(x, y);
//        int set = qRed(pixel) > 160 && qGreen(pixel) > 160 && qBlue(pixel) > 128;
        int set = (pixel & 0x00c0c080) == 0x00c0c080 ? 1 : 0;
        cell = set;
        if (cell == 0){
            image.setPixel(x, y, 0);
        }
        count += set;
    }
};




}
}
}
#endif
