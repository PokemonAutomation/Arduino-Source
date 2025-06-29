/*  Distance to Line
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *      Return a set of connected clusters.
 *
 */

#ifndef PokemonAutomation_CommonTools_DistanceToLine_H
#define PokemonAutomation_CommonTools_DistanceToLine_H

#include <cstddef>

namespace PokemonAutomation{


class DistanceToLine{
public:
    DistanceToLine(
        ptrdiff_t vertex0_x, ptrdiff_t vertex0_y,
        ptrdiff_t vertex1_x, ptrdiff_t vertex1_y
    )
        : m_vertical(vertex0_x == vertex1_x)
        , m_vertex0_x(vertex0_x)
        , m_vertex0_y(vertex0_y)
//        , m_vertex1_x(vertex1_x)
//        , m_vertex1_y(vertex1_y)
        , m_slope((double)(vertex1_y - vertex0_y) / (vertex1_x - vertex0_x))
        , m_inverse(1 / (m_slope*m_slope + 1))
    {}

    double distance_squared(ptrdiff_t x, ptrdiff_t y) const{
        if (m_vertical){
            ptrdiff_t distance = x - m_vertex0_x;
            return (double)(distance * distance);
        }
        ptrdiff_t dx = x - m_vertex0_x;
        ptrdiff_t dy = y - m_vertex0_y;
        double top = m_slope * dx - dy;
        double distance_sqr = top * top * m_inverse;
        return distance_sqr;
    }

private:
    bool m_vertical;
    ptrdiff_t m_vertex0_x;
    ptrdiff_t m_vertex0_y;
//    ptrdiff_t m_vertex1_x;
//    ptrdiff_t m_vertex1_y;
    double m_slope;
    double m_inverse;
};



}
#endif
