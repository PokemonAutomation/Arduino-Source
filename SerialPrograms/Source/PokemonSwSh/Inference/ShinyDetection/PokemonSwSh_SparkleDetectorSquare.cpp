/*  Square Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <map>
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonTools/Images/DistanceToLine.h"
#include "PokemonSwSh_SparkleDetectorSquare.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Kernels;
using namespace Kernels::Waterfill;


//  Remove the background of 1's from "matrix".
//  Return the background itself.
WaterfillObject remove_background(PackedBinaryMatrix& matrix){
    size_t width = matrix.width();
    size_t height = matrix.height();

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);

    WaterfillObject background;
    for (size_t c = 0; c < width; c++){
        WaterfillObject object;
        if (session->find_object_on_bit(object, true, c, 0)){
            background.merge_assume_no_overlap(object);
        }
        if (session->find_object_on_bit(object, true, c, height - 1)){
            background.merge_assume_no_overlap(object);
        }
    }
    for (size_t r = 0; r < height; r++){
        WaterfillObject object;
        if (session->find_object_on_bit(object, true, 0, r)){
            background.merge_assume_no_overlap(object);
        }
        if (session->find_object_on_bit(object, true, width - 1, r)){
            background.merge_assume_no_overlap(object);
        }
    }
    return background;
}


//  Remove the hole of 1's from "matrix" at location (x, y).
//  Return the total area of the hole.
size_t remove_hole(PackedBinaryMatrix& matrix, size_t x, size_t y){
    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    WaterfillObject object;
    if (!session->find_object_on_bit(object, false, x, y)){
        return 0;
    }
    return object.area;
}


//  Check if the object has a valid hole characteristic of a square.
bool check_hole(
    WaterfillObject& background,
    const PackedBinaryMatrix& object,
    size_t box_area, size_t object_area,
    size_t center_x, size_t center_y
){
    PackedBinaryMatrix inverted = object.copy();
    inverted.invert();

    //  Edge area is too large relative to the entire enclosing box.
    background = remove_background(inverted);
//    cout << "background_area = " << background.area << endl;
//    if (background_area * 3 > box_area * 2){
//        return false;
//    }

    //  Hole is too small or doesn't exist.
    size_t hole_area = remove_hole(inverted, center_x, center_y);
//    cout << "hole_area = " << hole_area << endl;
    if (hole_area < 20){
        return false;
    }

    //  Non-center hole area is more than 1/4 of the total hole area.
    size_t total_hole_area = box_area - background.area - object_area;
    size_t non_center_hole_area = total_hole_area - hole_area;
    if (non_center_hole_area * 4 > total_hole_area){
//        cout << "bad hole" << endl;
        return false;
    }

    return true;
}


//  Given object of 1's, zero all, but the locally furthest points.
//  Return the list of locally furthest points.
struct Point2{
    size_t x = 0;
    size_t y = 0;
    double distance = 0;
    double angle = 0;
    bool remove = false;

    bool operator==(const Point2& point) const{
        return x == point.x && y == point.y;
    }
};
std::vector<Point2> keep_furthest_points(const PackedBinaryMatrix& matrix, size_t center_x, size_t center_y){
    size_t width = matrix.width();
    size_t height = matrix.height();

    std::vector<Point2> points;
    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            //  Not part of the object.
            if (!matrix.get(c, r)){
                continue;
            }

            size_t my_dx2 = c - center_x;
            size_t my_dy2 = r - center_y;
            my_dx2 *= my_dx2;
            my_dy2 *= my_dy2;

            {
                size_t dx2 = c + 1 - center_x;
                dx2 *= dx2;
                if (dx2 > my_dx2 && c + 1 < width && matrix.get(c + 1, r)){
                    continue;
                }
            }
            {
                size_t dx2 = c - 1 - center_x;
                dx2 *= dx2;
                if (dx2 > my_dx2 && c > 0 && matrix.get(c - 1, r)){
                    continue;
                }
            }
            {
                size_t dy2 = r + 1 - center_y;
                dy2 *= dy2;
                if (dy2 > my_dy2 && r + 1 < height && matrix.get(c, r + 1)){
                    continue;
                }
            }
            {
                size_t dy2 = r - 1 - center_y;
                dy2 *= dy2;
                if (dy2 > my_dy2 && r > 0 && matrix.get(c, r - 1)){
                    continue;
                }
            }

            size_t my_distance = my_dx2 + my_dy2;
            {
                size_t dx2 = c + 1 - center_x;
                size_t dy2 = r + 1 - center_y;
                dx2 *= dx2;
                dy2 *= dy2;
                if (dx2 + dy2 > my_distance && c + 1 < width && r + 1 < height && matrix.get(c + 1, r + 1)){
                    continue;
                }
            }
            {
                size_t dx2 = c - 1 - center_x;
                size_t dy2 = r + 1 - center_y;
                dx2 *= dx2;
                dy2 *= dy2;
                if (dx2 + dy2 > my_distance && c > 0 && r + 1 < height && matrix.get(c - 1, r + 1)){
                    continue;
                }
            }
            {
                size_t dx2 = c + 1 - center_x;
                size_t dy2 = r - 1 - center_y;
                dx2 *= dx2;
                dy2 *= dy2;
                if (dx2 + dy2 > my_distance && c + 1 < width && r > 0 && matrix.get(c + 1, r - 1)){
                    continue;
                }
            }
            {
                size_t dx2 = c - 1 - center_x;
                size_t dy2 = r - 1 - center_y;
                dx2 *= dx2;
                dy2 *= dy2;
                if (dx2 + dy2 > my_distance && c > 0 && r > 0 && matrix.get(c - 1, r - 1)){
                    continue;
                }
            }
            points.emplace_back(
                Point2{c, r, std::sqrt(my_distance), 0, false}
            );
        }
    }
    return points;
}


//  Merge points that are close together - keeping the one that is further away.
void merge_nearby_points(std::map<double, Point2> points, double min_distance_diff = 0.2){
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
        Point2* last = &iter->second;
        ++iter;
        for (; iter != points.end(); ++iter){
            Point2* current = &iter->second;
            size_t dx = current->x - last->x;
            size_t dy = current->y - last->y;
            uint64_t distance = dx*dx + dy*dy;
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
            Point2* current = &points.begin()->second;
            size_t dx = current->x - last->x;
            size_t dy = current->y - last->y;
            uint64_t distance = dx*dx + dy*dy;
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


//  Find the corner that is opposite to the specified corner.
//  Returns false if no such corner is found.
bool find_opposite_corner(
    const std::map<double, Point2>& points_by_angle,
    const Point2& source_corner,
    Point2& opposite_corner
){
    double best_opposite = 0;
    for (const auto& point : points_by_angle){
        double diff = point.second.angle - source_corner.angle;
        if (diff >= 180){
            diff -= 360;
        }
        if (diff <= -180){
            diff += 360;
        }
        diff = std::abs(diff);
        if (best_opposite < diff){
            best_opposite = diff;
            opposite_corner = point.second;
        }
    }
    if (best_opposite < 160){
//        cout << "bad angle = " << best_opposite << endl;
        return false;
    }
    return true;
}


//
//  An fmod() variant that reduces to [0, 360) degrees.
//
double normalize_angle_0_360_new(double angle){
    while (angle < 0){
        angle += 360;
    }
    while (angle >= 360){
        angle -= 360;
    }
    return angle;
}


//  Get all the pixels that border the background.
std::multimap<double, std::pair<size_t, size_t>> get_edge_pixels(
    const PackedBinaryMatrix& object,
    const WaterfillObject& background,
    size_t center_x, size_t center_y,
    double base_angle
){
    size_t width = object.width();
    size_t height = object.height();

    std::multimap<double, std::pair<size_t, size_t>> edge_pixels;
    if (background.area == 0){
        //  No background. Grab the entire edge.
        for (size_t r = 0; r < height; r++){
            for (size_t c = 0; c < width; c++){
                //  Not part of the object.
                if (!object.get(c, r)){
                    continue;
                }
                if (c == 0 || r == 0 || c == width - 1 || r == height - 1){
                    double angle = std::atan2(
                        (ptrdiff_t)r - (ptrdiff_t)center_y,
                        (ptrdiff_t)c - (ptrdiff_t)center_x
                    ) * 57.295779513082320877;
                    angle = normalize_angle_0_360_new(angle - base_angle);
                    edge_pixels.emplace(
                        angle,
                        std::pair<size_t, size_t>{c, r}
                    );
                }
            }
        }
        return edge_pixels;
    }

    PackedBinaryMatrix background_matrix = background.object->submatrix(0, 0, width, height);
    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            //  Not part of the object.
            if (!object.get(c, r)){
                continue;
            }
            if (c == 0 || r == 0 || c == width - 1 || r == height - 1 ||
                background_matrix.get(c - 1, r) ||
                background_matrix.get(c + 1, r) ||
                background_matrix.get(c, r - 1) ||
                background_matrix.get(c, r + 1)
            ){
                double angle = std::atan2(
                    (ptrdiff_t)r - (ptrdiff_t)center_y,
                    (ptrdiff_t)c - (ptrdiff_t)center_x
                ) * 57.295779513082320877;
                angle = normalize_angle_0_360_new(angle - base_angle);
                edge_pixels.emplace(
                    angle,
                    std::pair<size_t, size_t>{c, r}
                );
            }
        }
    }
    return edge_pixels;
}


double area_of_triangle(
    ptrdiff_t x0, ptrdiff_t y0,
    ptrdiff_t x1, ptrdiff_t y1,
    ptrdiff_t x2, ptrdiff_t y2
){
    return ((double)(x0 - x2) * (y1 - y0) - (double)(x0 - x1) * (y2 - y0)) * 0.5;
}


//
//  Given a set of points, "points_by_angle" with angles "point_lo_angle"
//  and "point_hi_angle", compute the sum of squares of distances of the
//  points to the line that intersects:
//      [point_lo_x, point_lo_y]
//      [point_hi_x, point_hi_y]
//
double sum_squares_from_line(
    const std::multimap<double, std::pair<size_t, size_t>>& points_by_angle,
    double point_lo_angle, size_t point_lo_x, size_t point_lo_y,
    double point_hi_angle, size_t point_hi_x, size_t point_hi_y
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
        double dist_sqr = calc.distance_squared(iter0->second.first, iter0->second.second);
//        cout << "(" << iter0->second.first << "," << iter0->second.second << "), dist_sqr = " << dist_sqr << endl;
        sum_sqr += dist_sqr;
    }
//    cout << "average distance = " << sum / count << endl;
    return sum_sqr;
}



bool is_square_sparkle(const Kernels::Waterfill::WaterfillObject& object, double max_deviation){
    size_t width = object.width();
    size_t height = object.height();
//    cout << "width  = " << width << endl;
//    cout << "height = " << height << endl;
    if (width < 10 || height < 10){
        return false;
    }

    size_t box_area = width * height;
    const PackedBinaryMatrix matrix = object.packed_matrix();

    size_t center_x = (size_t)object.center_of_gravity_x() - object.min_x;
    size_t center_y = (size_t)object.center_of_gravity_y() - object.min_y;


    //  Check the hole.
    WaterfillObject background;
    if (!check_hole(background, matrix, box_area, object.area, center_x, center_y)){
        return false;
    }

    std::vector<Point2> corner_candidates = keep_furthest_points(matrix, center_x, center_y);
//    cout << matrix.dump() << endl;
//    for (const Point2& point : corner_candidates){
//        cout << "(" << point.x << ", " << point.y << ")" << endl;
//    }

    //  Too few points.
    if (corner_candidates.size() < 4){
//        cout << "too few points" << endl;
        return false;
    }


    //  Calculate angles.
    Point2 far_corner0;
    std::map<double, Point2> points_by_angle;
    {
        for (auto& point : corner_candidates){
            double angle = std::atan2(
                (int64_t)(point.y - center_y),
                (int64_t)(point.x - center_x)
            ) * 57.295779513082320877;

            point.angle = angle;
            points_by_angle.emplace(angle, point);
            if (far_corner0.distance < point.distance){
                far_corner0 = point;
            }
//            cout << angle << " : distance = " << point.distance << ": " << point.x << ", " << point.y << endl;
        }
        if (points_by_angle.size() < 4){
            return false;
        }
        corner_candidates.clear();
    }


    //  Merge points that are close together.
    merge_nearby_points(points_by_angle);
    if (points_by_angle.size() < 4){
        return false;
    }

#if 0
    for (const auto& point : points_by_angle){
        cout << point.first
             << " : distance = " << point.second.distance
             << ": " << point.second.x << ", " << point.second.y << endl;
    }
#endif


    //  Look for a point opposite to the furthest.
    Point2 far_corner1;
    if (!find_opposite_corner(points_by_angle, far_corner0, far_corner1)){
        return false;
    }
    double far_corner1_angle = normalize_angle_0_360_new(far_corner1.angle - far_corner0.angle);
//    cout << "far_corner1_angle = " << far_corner1_angle << endl;


    //  Build sets of points on each edge.
    std::map<double, Point2> edge0;
    std::map<double, Point2> edge1;
    for (const auto& point : points_by_angle){
        //  Skip the two far corners.
        if (point.second == far_corner0 || point.second == far_corner1){
            continue;
        }
        double diff = normalize_angle_0_360_new(point.second.angle - far_corner0.angle);
        if (diff < far_corner1_angle){
            edge0[diff] = point.second;
        }else{
            edge1[diff] = point.second;
        }
    }


    //  Get all the edge pixels.
    std::multimap<double, std::pair<size_t, size_t>> edge_pixels = get_edge_pixels(
        matrix, background,
        center_x, center_y, far_corner0.angle
    );
    if (edge_pixels.empty()){
        return false;
    }
//    cout << "edge_pixels.size() = " << edge_pixels.size() << endl;
//    for (const auto& item : points_by_angle){
//        cout << item.first << " : distance = " << item.second.distance << ", (" << item.second.x << "," << item.second.y << ")" << endl;
//    }


    PackedBinaryMatrix test(width, height);
    for (const auto& item : edge_pixels){
        test.set(item.second.first, item.second.second, true);
    }
//    cout << test.dump() << endl;


    //  Attempt to find linear regression fit.
    double size_scaling = (double)1 / (width + height);
    for (const auto& near_corner0 : edge0){
        double lo = std::max(near_corner0.first + 160, far_corner1_angle);
        double hi = std::min(near_corner0.first + 200, 360.);

        auto near_corner1_iter0 = edge1.lower_bound(lo);
        auto near_corner1_iter1 = edge1.upper_bound(hi);
        if (near_corner1_iter0 == near_corner1_iter1){
            //  No compatible corner.
            continue;
        }
//        cout << near_corner1_iter0->first
//             << " : distance = " << near_corner1_iter0->second.distance
//             << ", (" << near_corner1_iter0->second.x
//             << "," << near_corner1_iter0->second.y << ")" << endl;

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
//        cout << "sum_edge0 = " << sum_edge0 << endl;
//        cout << "sum_edge1 = " << sum_edge1 << endl;
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
            size_t actual_object_area = width * height - background.area;
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

    return false;
}




bool is_line_sparkle(const Kernels::Waterfill::WaterfillObject& object, size_t min_pixel_width){
    size_t width = object.width();
    size_t height = object.height();
    if (width < 5 || height < 5){
        return false;
    }
    if (width < min_pixel_width){
        return false;
    }
    if ((size_t)width * width < object.area * 50){
        return false;
    }
    if (width < height * 10){
        return false;
    }

    PackedBinaryMatrix matrix = object.packed_matrix();

    //  Make sure it's actually a line.

    std::vector<size_t> heights(width);
    bool row_ok = false;
//    cout << "{";
    for (size_t r = 0; r < matrix.height(); r++){
//        cout << "    {";
        size_t length = 0;
        size_t longest = 0;
        for (size_t c = 0; c < matrix.width(); c++){
//            cout << matrix[r][c] << ", ";
            if (matrix.get(c, r)){
                length++;
                longest = std::max(longest, length);
                heights[c]++;
            }else{
                length = 0;
            }
        }
//        cout << "}," << endl;
        if (length >= min_pixel_width){
            row_ok = true;
        }
    }
//    cout << "}" << endl;
    if (!row_ok){
        return false;
    }

    //  Make sure the bulge is not on the edge.
    size_t thickest = 0;
    size_t thickest_column = 0;
    for (size_t c = 0; c < width; c++){
        size_t h = heights[c];
//        cout << h << ", ";
        if (thickest < h){
            thickest = h;
            thickest_column = c;
        }
    }

    size_t diff_left = thickest_column;
    size_t diff_right = width - thickest_column;
//    cout << diff_left << " : " << diff_right << endl;
    if (diff_left * 10 < diff_right){
//        cout << "line is off-center" << endl;
        return false;
    }
    if (diff_right * 10 < diff_left){
//        cout << "line is off-center" << endl;
        return false;
    }

//    cout << width << " x " << height << endl;

    return true;
}



}
}
}
