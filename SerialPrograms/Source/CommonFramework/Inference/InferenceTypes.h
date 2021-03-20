/*  Inference Data Types
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_InferenceTypes_H
#define PokemonAutomation_CommonFramework_InferenceTypes_H

#include <QColor>

namespace PokemonAutomation{


struct PixelBox{
    int min_x;
    int min_y;
    int max_x;
    int max_y;

    int width() const{ return max_x - min_x; }
    int height() const{ return max_y - min_y; }
};


struct InferenceBox{
    QColor color;
    double x;
    double y;
    double width;
    double height;

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
