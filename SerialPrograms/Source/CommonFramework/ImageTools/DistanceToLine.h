/*  Distance to Line
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Return a set of connected clusters.
 *
 */

#ifndef PokemonAutomation_CommonFramework_DistanceToLine_H
#define PokemonAutomation_CommonFramework_DistanceToLine_H

#include "ImageBoxes.h"

namespace PokemonAutomation{


class DistanceToLine{
public:
    DistanceToLine(
        pxint_t vertex0_x, pxint_t vertex0_y,
        pxint_t vertex1_x, pxint_t vertex1_y
    )
        : m_vertical(vertex0_x == vertex1_x)
        , m_vertex0_x(vertex0_x)
        , m_vertex0_y(vertex0_y)
//        , m_vertex1_x(vertex1_x)
//        , m_vertex1_y(vertex1_y)
        , m_slope((double)(vertex1_y - vertex0_y) / (vertex1_x - vertex0_x))
        , m_inverse(1 / (m_slope*m_slope + 1))
    {}

    double distance_squared(pxint_t x, pxint_t y) const{
        if (m_vertical){
            double distance = (double)(x - m_vertex0_x);
            return distance * distance;
        }
        pxint_t dx = x - m_vertex0_x;
        pxint_t dy = y - m_vertex0_y;
        double top = m_slope * dx - dy;
        double distance_sqr = top * top * m_inverse;
        return distance_sqr;
    }

private:
    bool m_vertical;
    pxint_t m_vertex0_x;
    pxint_t m_vertex0_y;
//    pxint_t m_vertex1_x;
//    pxint_t m_vertex1_y;
    double m_slope;
    double m_inverse;
};



}
#endif
