/*  Inference Data Types
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_InferenceTypes_H
#define PokemonAutomation_CommonFramework_InferenceTypes_H

#include <QColor>

namespace PokemonAutomation{

using pxint_t = int;


struct PixelBox{
    pxint_t min_x;
    pxint_t min_y;
    pxint_t max_x;
    pxint_t max_y;

    pxint_t width() const{ return max_x - min_x; }
    pxint_t height() const{ return max_y - min_y; }
    size_t area() const{ return (size_t)width() * (size_t)height(); }
};


struct InferenceBox{
    QColor color;
    double x;
    double y;
    double width;
    double height;

    InferenceBox()
        : color(Qt::red)
        , x(0), y(0)
        , width(0), height(0)
    {}
    InferenceBox(
        double p_x, double p_y,
        double p_width, double p_height
    )
        : color(Qt::red)
        , x(p_x), y(p_y)
        , width(p_width), height(p_height)
    {}
    InferenceBox(
        QColor p_color,
        double p_x, double p_y,
        double p_width, double p_height
    )
        : color(p_color)
        , x(p_x), y(p_y)
        , width(p_width), height(p_height)
    {}
};



}
#endif
