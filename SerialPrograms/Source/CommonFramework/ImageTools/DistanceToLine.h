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
        double vertex0_x, double vertex0_y,
        double vertex1_x, double vertex1_y
    )
        : m_vertical(vertex0_x == vertex1_x)
        , m_vertex0_x(vertex0_x)
        , m_vertex0_y(vertex0_y)
//        , m_vertex1_x(vertex1_x)
//        , m_vertex1_y(vertex1_y)
        , m_slope((vertex1_y - vertex0_y) / (vertex1_x - vertex0_x))
        , m_inverse(1 / (m_slope*m_slope + 1))
    {}

    double distance_squared(double x, double y) const{
        if (m_vertical){
            double distance = x - m_vertex0_x;
            return distance * distance;
        }
        double dx = x - m_vertex0_x;
        double dy = y - m_vertex0_y;
        double top = m_slope * dx - dy;
        double distance_sqr = top * top * m_inverse;
        return distance_sqr;
    }

private:
    bool m_vertical;
    double m_vertex0_x;
    double m_vertex0_y;
//    double m_vertex1_x;
//    double m_vertex1_y;
    double m_slope;
    double m_inverse;
};



}
#endif
