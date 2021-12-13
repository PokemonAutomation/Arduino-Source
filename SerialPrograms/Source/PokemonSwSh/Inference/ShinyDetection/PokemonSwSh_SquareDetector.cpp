/*  Square Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <map>
#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/DistanceToLine.h"
#include "PokemonSwSh_SquareDetector.h"

#include <iostream>
using std::cout;
using std::endl;


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


struct Point{
    pxint_t x;
    pxint_t y;
    double distance;
    double angle;
    bool remove;
};

void dump_matrix(
    CellMatrix submatrix,
    pxint_t center_x, pxint_t center_y,
    const std::vector<Point>& points
){
    submatrix[center_y][center_x] = 9;
    for (const Point& item : points){
        submatrix[item.y][item.x] = 9;
    }
    cout << submatrix.dump() << endl;
}
template <typename MapType>
void dump_matrix(
    CellMatrix submatrix,
    pxint_t center_x, pxint_t center_y,
    const MapType& points_by_distance
){
    submatrix[center_y][center_x] = 9;
    for (const auto& item : points_by_distance){
        submatrix[item.second.y][item.second.x] = 9;
    }
    cout << submatrix.dump() << endl;
}


//
//  Make sure the square has a hole and that the center of the square is in
//  the hole.
//
bool check_hole(
    size_t& background_area,
    CellMatrix& submatrix,
    size_t object_area,
    pxint_t object_center_x,
    pxint_t object_center_y
){
    pxint_t width = submatrix.width();
    pxint_t height = submatrix.height();
    size_t box_area = (size_t)width * height;

    //  Fill in the edges.
    size_t edge_area = 0;
    for (pxint_t c = 0; c < width; c++){
        FillGeometry body;
        if (fill_geometry(body, submatrix, 1, c, 0, false, 2)){
            edge_area += body.area;
        }
        if (fill_geometry(body, submatrix, 1, c, height - 1, false, 2)){
            edge_area += body.area;
        }
    }
    for (pxint_t r = 0; r < height; r++){
        FillGeometry body;
        if (fill_geometry(body, submatrix, 1, 0, r, false, 2)){
            edge_area += body.area;
        }
        if (fill_geometry(body, submatrix, 1, width - 1, r, false, 2)){
            edge_area += body.area;
        }
    }
    background_area = edge_area;

    //  Edge area is too large relative to the entire enclosing box.
    if (edge_area * 3 > box_area * 2){
        return false;
    }

    //  The center of the object should be in the hole.
    FillGeometry hole;
    if (!fill_geometry(hole, submatrix, 1, object_center_x, object_center_y, false, 3)){
//        cout << "no hole in center" << endl;
        return false;
    }
    if (hole.area < 20){
        return false;
    }

    //  Non-center hole area is more than 1/4 of the total hole area.
    size_t total_hole_area = box_area - edge_area - object_area;
    size_t non_center_hole_area = total_hole_area - hole.area;
    if (non_center_hole_area * 4 > total_hole_area){
        return false;
    }

    return true;
}




//
//  Get a set of points that are locally the furthest away from the center of
//  the object.
//
std::vector<Point> get_furthest_points(
    const CellMatrix& submatrix,
    pxint_t center_x, pxint_t center_y
){
    pxint_t width = submatrix.width();
    pxint_t height = submatrix.height();
//    cout << "width  = " << width << endl;
//    cout << "height = " << height << endl;

    //  Keep points that are locally furthest away from the center.
    std::vector<Point> points;
    for (pxint_t r = 0; r < height; r++){
        for (pxint_t c = 0; c < width; c++){
            CellMatrix::ObjectID cell = submatrix[r][c];
            if (cell != 0){
                continue;
            }

            pxint_t my_dx2 = c - center_x;
            pxint_t my_dy2 = r - center_y;
            my_dx2 *= my_dx2;
            my_dy2 *= my_dy2;

            {
                pxint_t dx2 = c + 1 - center_x;
                dx2 *= dx2;
                if (dx2 > my_dx2 && c + 1 < width && submatrix[r][c + 1] == 0){
                    continue;
                }
            }
            {
                pxint_t dx2 = c - 1 - center_x;
                dx2 *= dx2;
                if (dx2 > my_dx2 && c > 0 && submatrix[r][c - 1] == 0){
                    continue;
                }
            }
            {
                pxint_t dy2 = r + 1 - center_y;
                dy2 *= dy2;
                if (dy2 > my_dy2 && r + 1 < height && submatrix[r + 1][c] == 0){
                    continue;
                }
            }
            {
                pxint_t dy2 = r - 1 - center_y;
                dy2 *= dy2;
                if (dy2 > my_dy2 && r > 0 && submatrix[r - 1][c] == 0){
                    continue;
                }
            }

            pxint_t my_distance = my_dx2 + my_dy2;
            {
                pxint_t dx2 = c + 1 - center_x;
                pxint_t dy2 = r + 1 - center_y;
                dx2 *= dx2;
                dy2 *= dy2;
                if (dx2 + dy2 > my_distance && c + 1 < width && r + 1 < height && submatrix[r + 1][c + 1] == 0){
                    continue;
                }
            }
            {
                pxint_t dx2 = c - 1 - center_x;
                pxint_t dy2 = r + 1 - center_y;
                dx2 *= dx2;
                dy2 *= dy2;
                if (dx2 + dy2 > my_distance && c > 0 && r + 1 < height && submatrix[r + 1][c - 1] == 0){
                    continue;
                }
            }
            {
                pxint_t dx2 = c + 1 - center_x;
                pxint_t dy2 = r - 1 - center_y;
                dx2 *= dx2;
                dy2 *= dy2;
                if (dx2 + dy2 > my_distance && c + 1 < width && r > 0 && submatrix[r - 1][c + 1] == 0){
                    continue;
                }
            }
            {
                pxint_t dx2 = c - 1 - center_x;
                pxint_t dy2 = r - 1 - center_y;
                dx2 *= dx2;
                dy2 *= dy2;
                if (dx2 + dy2 > my_distance && c > 0 && r > 0 && submatrix[r - 1][c - 1] == 0){
                    continue;
                }
            }

//            double angle = std::atan2(r - center_y, c - center_x);
            points.emplace_back(
                Point{c, r, std::sqrt(my_distance), std::nan(""), false}
            );
//            cout << std::sqrt(my_distance) << " : [" << c << "," << r << "]" << endl;
        }
    }
    return points;
}


//
//  Given the output of "get_furthest_points()", attempt to merge the ones that
//  are close together - keeping the one that is further away.
//
void merge_nearby_points(std::map<double, Point>& points, double min_distance_diff = 0.2){
    //  Merge points that are close together.
    const int DISTANCE_THRESHOLD = 5 * 5;
    bool changed;
    do{
        if (points.size() < 4){
            return;
        }

        //  Iterate adjacent points and merge points that are close together.
        changed = false;
        auto iter = points.begin();
        Point* last = &iter->second;
        ++iter;
        for (; iter != points.end(); ++iter){

            Point* current = &iter->second;
            pxint_t dx = current->x - last->x;
            pxint_t dy = current->y - last->y;
            uint64_t distance = (int64_t)dx*dx + (int64_t)dy*dy;
            if (distance < DISTANCE_THRESHOLD){
                if (current->distance + min_distance_diff < last->distance){
                    current->remove = true;
                    changed = true;
                }
                if (current->distance > last->distance + min_distance_diff){
                    last->remove = true;
                    changed = true;
                }
            }
            last = current;
        }
        {
            Point* current = &points.begin()->second;
            pxint_t dx = current->x - last->x;
            pxint_t dy = current->y - last->y;
            uint64_t distance = (int64_t)dx*dx + (int64_t)dy*dy;
            if (distance < DISTANCE_THRESHOLD){
                if (current->distance + min_distance_diff < last->distance){
                    current->remove = true;
                    changed = true;
                }
                if (current->distance > last->distance + min_distance_diff){
                    last->remove = true;
                    changed = true;
                }
            }
//            last = current;
        }

        //  Remove points marked for removal.
        for (iter = points.begin(); iter != points.end();){
            if (iter->second.remove){
//                cout << "remove" << endl;
                iter = points.erase(iter);
            }else{
                ++iter;
            }
        }
//        cout << "asdf" << endl;
    }while (changed);
}


//
//  An fmod() variant that reduces to [0, 360) degrees.
//
double normalize_angle_0_360(double angle){
    while (angle < 0){
        angle += 360;
    }
    while (angle >= 360){
        angle -= 360;
    }
    return angle;
}


double area_of_triangle(
    pxint_t x0, pxint_t y0,
    pxint_t x1, pxint_t y1,
    pxint_t x2, pxint_t y2
){
    return ((double)(x0 - x2) * (y1 - y0) - (double)(x0 - x1) * (y2 - y0)) * 0.5;
}


//
//  Return a set of all pixels that border the background.
//
//  The returned map is keyed by the angle of the point to the center of the
//  object. This angle is relative to "base_angle" and reduced [0, 360).
//
std::multimap<double, std::pair<pxint_t, pxint_t>> get_edge_points(
    const CellMatrix& submatrix,
    pxint_t center_x, pxint_t center_y, double base_angle,
    CellMatrix::ObjectID object,
    CellMatrix::ObjectID background
){
    pxint_t width = submatrix.width();
    pxint_t height = submatrix.height();

    std::multimap<double, std::pair<pxint_t, pxint_t>> ret;
    for (pxint_t r = 0; r < height; r++){
        for (pxint_t c = 0; c < width; c++){
            if (submatrix[r][c] != object){
                continue;
            }
            if ((c     > 0      && submatrix[r][c - 1] == background) ||
                (c + 1 < width  && submatrix[r][c + 1] == background) ||
                (r     > 0      && submatrix[r - 1][c] == background) ||
                (r + 1 < height && submatrix[r + 1][c] == background)
            ){
                double angle = std::atan2(
                    r - center_y,
                    c - center_x
                ) * 57.295779513082320877;
                angle = normalize_angle_0_360(angle - base_angle);
                ret.emplace(
                    angle,
                    std::pair<pxint_t, pxint_t>{c, r}
                );
            }
        }
    }

    return ret;
}


//
//  Given a set of points, "points_by_angle" with angles "point_lo_angle"
//  and "point_hi_angle", compute the sum of squares of distances of the
//  points to the line that intersects:
//      [point_lo_x, point_lo_y]
//      [point_hi_x, point_hi_y]
//
double sum_squares_from_line(
    const std::multimap<double, std::pair<pxint_t, pxint_t>>& points_by_angle,
    double point_lo_angle, pxint_t point_lo_x, pxint_t point_lo_y,
    double point_hi_angle, pxint_t point_hi_x, pxint_t point_hi_y
){
//    cout << "Point 0: angle = " << point_lo_angle << ", [" << point_lo_x << "," << point_lo_y << "]" << endl;
//    cout << "Point 1: angle = " << point_hi_angle << ", [" << point_hi_x << "," << point_hi_y << "]" << endl;

//    size_t count = 0;
//    double sum = 0;
    double sum_sqr = 0;

    auto iter0 = points_by_angle.lower_bound(point_lo_angle);
    auto iter1 = points_by_angle.lower_bound(point_hi_angle);
    DistanceToLine calc(
        point_lo_x, point_lo_y,
        point_hi_x, point_hi_y
    );
    for (; iter0 != iter1; ++iter0){
        sum_sqr += calc.distance_squared(iter0->second.first, iter0->second.second);
    }
//    cout << "average distance = " << sum / count << endl;
    return sum_sqr;
}


//
//  Return true if the object is likely a square.
//
bool is_square2(
    const QImage& image,
    const CellMatrix& matrix,
    const FillGeometry& object,
    double max_deviation
){
//    static int c = 0;
//    image.copy(object.box.min_x, object.box.min_y, object.box.width(), object.box.height()).save("item-" + QString::number(++c) + ".png");

    pxint_t width = object.box.width();
    pxint_t height = object.box.height();
//    cout << "width  = " << width << endl;
//    cout << "height = " << height << endl;
    if (width < 10 || height < 10){
        return false;
    }
    if (object.area * 10 > (size_t)matrix.height() * matrix.width()){
//        cout << "Bad area." << endl;
        return false;
    }
//    size_t box_area = (size_t)width * height;

    CellMatrix submatrix = matrix.extract(object.box, object.id);
    pxint_t center_x = object.center_x() - object.box.min_x;
    pxint_t center_y = object.center_y() - object.box.min_y;

    //  Invert the cells.
    for (pxint_t r = 0; r < height; r++){
        for (pxint_t c = 0; c < width; c++){
            CellMatrix::ObjectID cell = submatrix[r][c];
            cell = cell ? 0 : 1;
            submatrix[r][c] = cell;
        }
    }
//    cout << submatrix.dump() << endl;

//    static int c = 0;
//    image.copy(object.box.min_x, object.box.min_y, object.box.width(), object.box.height()).save("item-" + QString::number(++c) + ".png");

    //  Check the hole
    size_t background_area;
    if (!check_hole(background_area, submatrix, object.area, center_x, center_y)){
        return false;
    }
    //  The hole will be 1.
    //  The edge will be 2.
    //  The square will be 0.
//    cout << submatrix.dump() << endl;


    //  Keep points that are locally furthest away from the center.
    std::vector<Point> corner_candidates = get_furthest_points(submatrix, center_x, center_y);
    if (corner_candidates.size() < 4){
        return false;
    }

    const Point* best_point = nullptr;
    {
        double furthest = 0;
        for (const Point& point : corner_candidates){
            if (furthest < point.distance){
                furthest = point.distance;
                best_point = &point;
            }
        }
    }

//    dump_matrix(submatrix, center_x, center_y, corner_candidates);

    //  Too few points.
    if (corner_candidates.size() < 4){
//        cout << "too few points" << endl;
        return false;
    }

    //  Calculate angles.
    std::map<double, Point> points_by_angle;
    for (auto& point : corner_candidates){
        double angle = std::atan2(
            point.y - center_y,
            point.x - center_x
        ) * 57.295779513082320877;

        point.angle = angle;
        points_by_angle[angle] = point;
//        cout << "distance = " << point.first << ": " << point.second.x << ", " << point.second.y << endl;
    }
    if (points_by_angle.size() < 4){
        return false;
    }
    Point far_corner0 = *best_point;
    corner_candidates.clear();


    //  Merge points that are close together.
    merge_nearby_points(points_by_angle);
    if (points_by_angle.size() < 4){
        return false;
    }

//    dump_matrix(submatrix, center_x, center_y, points_by_angle);
//    cout << "points = " << points_by_angle.size() << endl;


    //  Look for a point opposite to the furthest.
    {
        double best_opposite = 0;
        for (const auto& point : points_by_angle){
            double diff = point.second.angle - far_corner0.angle;
            if (diff >= 180){
                diff -= 360;
            }
            if (diff <= -180){
                diff += 360;
            }
            diff = std::abs(diff);
            if (best_opposite < diff){
                best_opposite = diff;
                best_point = &point.second;
            }
        }
        if (best_opposite < 160){
//            cout << "bad angle = " << best_opposite << endl;
            return false;
        }
    }
//    cout << "qwer" << endl;


    Point far_corner1 = *best_point;
    double far_corner1_angle = normalize_angle_0_360(far_corner1.angle - far_corner0.angle);


    //  Build sets of points on each edge.
    std::map<double, Point> edge0;
    std::map<double, Point> edge1;
    {
        for (const auto& point : points_by_angle){
            if (&point.second == &far_corner0 || &point.second == &far_corner1){
                continue;
            }

            double diff = normalize_angle_0_360(point.second.angle - far_corner0.angle);
            if (diff < far_corner1_angle){
                edge0[diff] = point.second;
            }else{
                edge1[diff] = point.second;
            }
        }
    }


    //  Build array of all edge pixels.
    std::multimap<double, std::pair<pxint_t, pxint_t>> edge_pixels = get_edge_points(
        submatrix,
        center_x, center_y, far_corner0.angle,
        0, 2
    );
    if (edge_pixels.empty()){
        return false;
    }

//    dump_matrix(submatrix, center_x, center_y, points_by_angle);

//    double size_scaling = 1 / std::sqrt(width + height);
    double size_scaling = (double)1 / (width + height);

    //  Attempt to find a good fit.
    for (const auto& near_corner0 : edge0){
        double lo = std::max(near_corner0.first + 160, far_corner1_angle);
        double hi = std::min(near_corner0.first + 200, 360.);

        auto near_corner1_iter0 = edge1.lower_bound(lo);
        auto near_corner1_iter1 = edge1.upper_bound(hi);
        if (near_corner1_iter0 == near_corner1_iter1){
            //  No compatible corner.
            continue;
        }

        //  Calculate things on 2 of the edges.
        double sum_edge0 = sum_squares_from_line(
            edge_pixels,
            0, far_corner0.x, far_corner0.y,
            near_corner0.first, near_corner0.second.x, near_corner0.second.y
        );
        double sum_edge1 = sum_squares_from_line(
            edge_pixels,
            near_corner0.first, near_corner0.second.x, near_corner0.second.y,
            far_corner1_angle, far_corner1.x, far_corner1.y
        );
        double area0 = area_of_triangle(
            center_x, center_y,
            far_corner0.x, far_corner0.y,
            near_corner0.second.x, near_corner0.second.y
        );
        double area1 = area_of_triangle(
            center_x, center_y,
            near_corner0.second.x, near_corner0.second.y,
            far_corner1.x, far_corner1.y
        );

        //  Iterate through the candidate corners on the other side.
        for (; near_corner1_iter0 != near_corner1_iter1; ++near_corner1_iter0){
            double sum_edge2 = sum_squares_from_line(
                edge_pixels,
                far_corner1_angle, far_corner1.x, far_corner1.y,
                near_corner1_iter0->first, near_corner1_iter0->second.x, near_corner1_iter0->second.y
            );
            double sum_edge3 = sum_squares_from_line(
                edge_pixels,
                near_corner1_iter0->first, near_corner1_iter0->second.x, near_corner1_iter0->second.y,
                360, far_corner0.x, far_corner0.y
            );
            double area2 = area_of_triangle(
                center_x, center_y,
                far_corner1.x, far_corner1.y,
                near_corner1_iter0->second.x, near_corner1_iter0->second.y
            );
            double area3 = area_of_triangle(
                center_x, center_y,
                near_corner1_iter0->second.x, near_corner1_iter0->second.y,
                far_corner0.x, far_corner0.y
            );

            double sum_squares = sum_edge0 + sum_edge1 + sum_edge2 + sum_edge3;
            double normalized_distance = std::sqrt(sum_squares / edge_pixels.size());

            normalized_distance *= size_scaling;
//            cout << "normalized_distance = " << normalized_distance << endl;

            //  Too much deviation from edges.
            if (normalized_distance > max_deviation){
                continue;
            }

            double regression_area = area0 + area1 + area2 + area3;
            double actual_object_area = width * height - background_area;
//            cout << "area = " << regression_area << " / " << actual_object_area << endl;

            if (actual_object_area < regression_area){
                continue;
            }

//            cout << "image: " << c << endl;
//            cout << "normalized_distance = " << normalized_distance << endl;
//            dump_matrix(submatrix, center_x, center_y, points_by_angle);
            return true;
        }
    }

//    cout << "bad" << endl;
    return false;
}






}
}
}
