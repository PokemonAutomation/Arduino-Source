/*  Star Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <set>
#include "PokemonSwSh_ShinyFilters.h"
#include "PokemonSwSh_SparkleTrigger.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

const double SparkleDetector::ANGLE_TOLERANCE_DEGREES = 10.;


SparkleDetector::SparkleDetector(
    const FillMatrix& matrix,
    const FillGeometry& box,
    double percentile
)
    : m_box(box)
    , m_percentile(percentile)
{
    if (box.area < 20){
        return;
    }
    if (box.box.width() * box.box.height() > 10000){
//        return;
    }

    //  Copy the relevant portion of the matrix and keep only the current object.
    m_matrix = matrix.extract(box.box, box.id);

    //  Sort all pixels by distance from center.
    int center_x = box.center_x - box.box.min_x;
    int center_y = box.center_y - box.box.min_y;
    std::map<uint64_t, size_t> distances;
    for (size_t r = 0; r < m_matrix.height(); r++){
        for (size_t c = 0; c < m_matrix.width(); c++){
            if (m_matrix[r][c] != 0){
            int dist_x = (int)c - center_x;
            int dist_y = (int)r - center_y;
                uint64_t distance_sqr = (uint64_t)dist_x*dist_x + (uint64_t)dist_y*dist_y;
                distances[distance_sqr]++;
            }
        }
    }

    //  Filter out the bottom 80%.
    int stop = (int)(0.85 * box.area);
    int count = 0;
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
            int dist_x = (int)c - center_x;
            int dist_y = (int)r - center_y;
            uint64_t distance_sqr = (uint64_t)dist_x*dist_x + (uint64_t)dist_y*dist_y;
            if (distance_sqr < distance){
                m_matrix[r][c] = 0;
            }
        }
    }

    //  Find new regions.
    FillMatrix::ObjectID id = 2;
    for (size_t r = 0; r < m_matrix.height(); r++){
        for (size_t c = 0; c < m_matrix.width(); c++){
            FillGeometry region;
            if (!fill_geometry(region, m_matrix, 1, c, r, true, id)){
                continue;
            }
            id++;
//            if (region.area < 4){
//                continue;
//            }
            m_regions.emplace(-(ssize_t)region.area, region);
//            cout << region.center_x << ", " << region.center_y << " : " << region.area << endl;
        }
    }

#if 0
    cout << "{" << endl;
    for (size_t r = 0; r < m_matrix.height(); r++){
        cout << "    {";
        for (size_t c = 0; c < m_matrix.width(); c++){
            cout << m_matrix[r][c] << ",";
        }
        cout << "}" << endl;
    }
    cout << "}" << endl;
#endif
}

bool SparkleDetector::is_ball(){
    //  Fewer than 4 regions, cannot be a ball.
    if (m_regions.size() < 4){
        return false;
    }

    //  Make sure area is less than 1/2 of the box.
    int width = m_box.box.width();
    int height = m_box.box.height();
//    cout << "area = " << m_box.area * 2 << ", box = " << (uint64_t)width * height << endl;
    if (m_box.area * 2 > (uint64_t)width * height){
        return false;
    }

    //  Compute angles for the 4 largest regions.
    int center_x = m_box.center_x - m_box.box.min_x;
    int center_y = m_box.center_y - m_box.box.min_y;
    std::set<double> angles;
    size_t c = 0;
    for (const auto& region : m_regions){
        int x = region.second.center_x - center_x;
        int y = region.second.center_y - center_y;
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

#if 0
    cout << "{" << endl;
    for (size_t r = 0; r < m_matrix.height(); r++){
        cout << "    {";
        for (size_t c = 0; c < m_matrix.width(); c++){
            cout << m_matrix[r][c] << ",";
        }
        cout << "}" << endl;
    }
    cout << "}" << endl;
#endif

    //  Verify angles are aligned to the corners.
    for (double angle : angles){
        double n = angle - 45;
        double q = std::round(n / 90.);
        double m = n - 90. * q;
        if (std::abs(m) > ANGLE_TOLERANCE_DEGREES){
            return false;
        }
    }

    //  Verify that all live pixels are near the diagonals.
    for (size_t r = 0; r < m_matrix.height(); r++){
        for (size_t c = 0; c < m_matrix.width(); c++){
            if (m_matrix[r][c] == 0){
                continue;
            }
            double distance = std::sqrt(center_x*center_x + center_y*center_y);
            int dist_x = std::abs((int)c - center_x);
            int dist_y = std::abs((int)r - center_y);
            int dist = std::abs(dist_y - dist_x);
            if (dist > 2 + distance / 5.){
                return false;
            }
        }
    }

    return true;
}
bool SparkleDetector::is_star(){
    //  Fewer than 5 regions, cannot be a star.
    if (m_regions.size() < 5){
        return false;
    }

    int width = m_box.box.width();
    int height = m_box.box.height();

    //  Make sure dimentions are roughly square-ish.
    if (width > 2 * height || height > 2 * width){
        return false;
    }

    //  Compute angles for the 5 largest regions.
    int center_x = m_box.center_x - m_box.box.min_x;
    int center_y = m_box.center_y - m_box.box.min_y;
    std::set<double> angles;
    size_t c = 0;
    for (const auto& region : m_regions){
        int x = region.second.center_x - center_x;
        int y = region.second.center_y - center_y;
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
            if (std::abs(m) > ANGLE_TOLERANCE_DEGREES){
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

