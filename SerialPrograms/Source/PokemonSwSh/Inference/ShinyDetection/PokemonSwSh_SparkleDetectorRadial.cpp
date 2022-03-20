/*  Shiny Sparkle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <set>
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "PokemonSwSh_SparkleDetectorRadial.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Kernels;
using namespace Kernels::Waterfill;


const double STAR_SPARKLE_ANGLE_TOLERANCE_DEGREES = 10.;


RadialSparkleDetector::~RadialSparkleDetector(){}
RadialSparkleDetector::RadialSparkleDetector(const WaterfillObject& object)
    : m_object(object)
{
    if (object.area < 20){
        return;
    }
    if (object.area > 10000){
        return;
    }
    m_matrix = object.packed_matrix();
//    cout << m_matrix.dump() << endl;

    //  Sort all pixels by distance from center.
    size_t center_x = object.center_x() - object.min_x;
    size_t center_y = object.center_y() - object.min_y;
    std::map<uint64_t, size_t> distances;
    for (size_t r = 0; r < m_matrix.height(); r++){
        for (size_t c = 0; c < m_matrix.width(); c++){
            if (m_matrix.get(c, r)){
                size_t dist_x = c - center_x;
                size_t dist_y = r - center_y;
                uint64_t distance_sqr = (uint64_t)dist_x*dist_x + (uint64_t)dist_y*dist_y;
                distances[distance_sqr]++;
            }
        }
    }

    //  Filter out the bottom 85%.
    size_t stop = (size_t)(0.85 * object.area);
    size_t count = 0;
    uint64_t distance = 0;
    for (auto& item : distances){
        count += item.second;
        if (count >= stop){
            distance = item.first;
            break;
        }
    }
    m_radius_sqr = distance;
    for (size_t r = 0; r < m_matrix.height(); r++){
        for (size_t c = 0; c < m_matrix.width(); c++){
            size_t dist_x = c - center_x;
            size_t dist_y = r - center_y;
            uint64_t distance_sqr = (uint64_t)dist_x*dist_x + (uint64_t)dist_y*dist_y;
            if (distance_sqr < distance){
                m_matrix.set(c, r, false);
            }
        }
    }

    //  Find new regions.
    PackedBinaryMatrix2 matrix = m_matrix;
    auto finder = make_WaterfillIterator(matrix, 1);
    WaterfillObject obj;
    while (finder->find_next(obj)){
        obj.object.reset();
        m_regions.emplace(obj.area, std::move(obj));
    }

//    cout << m_matrix.dump() << endl;
}

bool RadialSparkleDetector::is_ball() const{
    //  Fewer than 4 regions, cannot be a ball.
    if (m_regions.size() < 4){
        return false;
    }

    size_t width = m_object.width();
    size_t height = m_object.height();

    //  Make sure area is less than 2/3 of the box.
//    cout << "area = " << m_box.area * 2 << ", box = " << (uint64_t)width * height << endl;
    if (m_object.area * 3 > (uint64_t)width * height * 2){
//        cout << "bad area" << endl;
        return false;
    }

    //  Compute angles for the 4 largest regions.
    pxint_t center_x = (pxint_t)(m_object.center_x() - m_object.min_x);
    pxint_t center_y = (pxint_t)(m_object.center_y() - m_object.min_y);
    std::set<double> angles;
    {
        size_t c = 0;
        for (const auto& region : m_regions){
            pxint_t x = (pxint_t)region.second.center_x() - center_x;
            pxint_t y = (pxint_t)region.second.center_y() - center_y;
            double angle = std::atan2(y, x) * 57.29577951308232;
            if (angle < 0){
                angle += 360;
            }
            angles.insert(angle);
            c++;
            if (c >= 4){
                break;
            }
        }
    }

    //  Verify angles are aligned to the corners.
    for (double angle : angles){
        double n = angle - 45;
        double q = std::round(n / 90.);
        double m = n - 90. * q;
        if (std::abs(m) > STAR_SPARKLE_ANGLE_TOLERANCE_DEGREES){
            return false;
        }
    }

    //  Verify that all live pixels are near the diagonals.
    for (size_t r = 0; r < m_matrix.height(); r++){
        for (size_t c = 0; c < m_matrix.width(); c++){
            if (!m_matrix.get(c, r)){
                continue;
            }
            double distance = std::sqrt(center_x*center_x + center_y*center_y);
            pxint_t dist_x = std::abs((pxint_t)(c - center_x));
            pxint_t dist_y = std::abs((pxint_t)(r - center_y));
            pxint_t dist = std::abs((pxint_t)(dist_y - dist_x));
            if (dist > 2 + distance / 5.){
                return false;
            }
        }
    }

    return true;
}
bool RadialSparkleDetector::is_star() const{
    //  Fewer than 5 regions, cannot be a star.
    if (m_regions.size() < 5){
        return false;
    }

    size_t width = m_object.width();
    size_t height = m_object.height();

    //  Make sure dimentions are roughly square-ish.
    if (width > 2 * height || height > 2 * width){
        return false;
    }

    //  Compute angles for the 5 largest regions.
    pxint_t center_x = (pxint_t)(m_object.center_x() - m_object.min_x);
    pxint_t center_y = (pxint_t)(m_object.center_y() - m_object.min_y);
    std::set<double> angles;
    size_t c = 0;
    for (const auto& region : m_regions){
        pxint_t x = (pxint_t)region.second.center_x() - center_x;
        pxint_t y = (pxint_t)region.second.center_y() - center_y;
        double angle = std::atan2(y, x) * 57.29577951308232;
        if (angle < 0){
            angle += 360;
        }
        angles.insert(angle);
        c++;
        if (c >= 5){
            break;
        }
    }

    //  Attempt best fit.
    bool match = false;
    for (double angle : angles){
        size_t bad = 0;
        uint8_t points = 0;
        for (double test : angles){
            double n = test - angle;
            double q = std::round(n / 72.);
            points |= (size_t)1 << ((size_t)q % 5);
            double m = n - 72. * q;
            if (std::abs(m) > STAR_SPARKLE_ANGLE_TOLERANCE_DEGREES){
                bad++;
            }
        }

        //  Not all 5 points are detected.
        if ((points & 0x1f) != 0x1f){
            continue;
        }

        //  Bad angles.
        if (bad <= 0){
            match = true;
            break;
        }
    }

    return match;
}






















}
}
}
