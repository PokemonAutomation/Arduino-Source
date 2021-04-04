/*  Square Shiny Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include <set>
#include "PokemonSwSh_SquareTrigger.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

const double SQUARE_ANGLE_TOLERANCE_DEGREES = 30;



bool is_square_beam(
    const FillMatrix& matrix,
    const FillGeometry& object,
    double min_length
){
    int width = object.box.width();
    int height = object.box.height();
    if (width < 5 || height < 5){
        return false;
    }
    int min_pixel_width = (int)(matrix.width() * min_length);
    if (width < min_pixel_width){
        return false;
    }
    if ((size_t)width * width < object.area * 50){
        return false;
    }
    if (width < height * 10){
        return false;
    }

    //  Make sure it's actually a line.

    std::vector<int> heights(width);
    bool row_ok = false;
//    cout << "{";
    for (int r = object.box.min_y; r < object.box.max_y; r++){
//        cout << "    {";
        int length = 0;
        for (int c = object.box.min_x; c < object.box.max_x; c++){
//            cout << matrix[r][c] << ", ";
            int ok = matrix[r][c] == object.id ? 1 : 0;
            length += ok;
            heights[c - object.box.min_x] += ok;
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
    int thickest = 0;
    int thickest_column = 0;
    for (int c = 0; c < width; c++){
        int h = heights[c];
//        cout << h << ", ";
        if (thickest < h){
            thickest = h;
            thickest_column = c;
        }
    }
//    cout << "}" << endl;
//    cout << "thickest = " << thickest << endl;
    int diff_left = thickest_column;
    int diff_right = width - thickest_column;
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


//
//  The old square detector. Deprecated.
//
#if 0
bool is_square(
    const FillMatrix& matrix,
    const FillGeometry& object
){
    int width = object.box.width();
    int height = object.box.height();
    if (width < 5 || height < 5){
//        cout << "too small" << endl;
        return false;
    }
    if (object.area * 10 > matrix.height() * matrix.width()){
        return false;
    }
//    cout << "asdf" << endl;
    size_t box_area = (size_t)width * height;
//    if (object.area * 2 > box_area){
//        return false;
//    }
//    return true;

//    cout << "qwer" << endl;

    FillMatrix submatrix = matrix.extract(object.box, object.id);
    int center_x = object.center_x - object.box.min_x;
    int center_y = object.center_y - object.box.min_y;

    //  Invert the cells.
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
            FillMatrix::ObjectID cell = submatrix[r][c];
            cell = cell ? 0 : 1;
            submatrix[r][c] = cell;
        }
    }

    //  Check the hole
    {
        //  Fill in the edges.
        size_t edge_area = 0;
        for (int c = 0; c < width; c++){
            FillGeometry body;
            if (fill_geometry(body, submatrix, 1, c, 0, false, 2)){
                edge_area += body.area;
            }
            if (fill_geometry(body, submatrix, 1, c, height - 1, false, 2)){
                edge_area += body.area;
            }
        }
        for (int r = 0; r < height; r++){
            FillGeometry body;
            if (fill_geometry(body, submatrix, 1, 0, r, false, 2)){
                edge_area += body.area;
            }
            if (fill_geometry(body, submatrix, 1, width - 1, r, false, 2)){
                edge_area += body.area;
            }
        }

        //  The center of the object should be in the hole.
        if (submatrix[center_y][center_x] != 1){
//            cout << "no hole in center" << endl;
            return false;
        }

        size_t hole_area = box_area - edge_area - object.area;

//        cout << "box = " << box_area << ", edge = " << edge_area << ", body = " << object.area
//             << ", hole = " << hole_area << endl;

        if (hole_area < 20 || edge_area * 3 > box_area * 2){
            return false;
        }
    }

//    cout << submatrix.dump() << endl;


    //  Check vertex angles.
    {
        submatrix = matrix.extract(object.box, object.id);

        //  Clear all but the edges.
        for (int r = 1; r < height - 1; r++){
            for (int c = 1; c < width - 1; c++){
                submatrix[r][c] = 0;
            }
        }
//        cout << submatrix.dump() << endl;

        std::vector<FillGeometry> regions = find_all_objects(submatrix, 1, false, 1);
//        cout << "regions = " << regions.size() << endl;

        //  Get furthest points.
        std::vector<std::pair<int, int>> points;
        for (const auto& region : regions){
            if (region.box.height() == 1){
                if (region.box.min_x <= center_x){
                    points.emplace_back(region.box.min_x, region.center_y);
                }
                if (region.box.max_x > center_x){
                    points.emplace_back(region.box.max_x, region.center_y);
                }
            }else{
                if (region.box.min_y <= center_y){
                    points.emplace_back(region.center_x, region.box.min_y);
                }
                if (region.box.max_y > center_y){
                    points.emplace_back(region.center_x, region.box.max_y);
                }
            }
        }

        //  Compute angles.
        std::set<double> angles;
        for (const auto& point : points){
            int x = point.first - center_x;
            int y = point.second - center_y;
            double angle = std::atan2(y, x) * 57.29577951308232;
            if (angle < 0){
                angle += 360;
            }
            angles.insert(angle);
//            cout << "angle = " << angle << ", [" << x << "," << y << "]" << endl;
        }

        //  Verify angles.
        std::map<double, int> angle_sets;
        while (!angles.empty()){
            double pivot = *angles.begin();
            int& high = angle_sets[pivot] = false;
            for (auto angle : angles){
                double n = angle - pivot;
                double q = std::round(n / 180.);
                double r = n - 180*q;
                if (std::abs(r) < SQUARE_ANGLE_TOLERANCE_DEGREES){
                    angles.erase(angle);
                    if (n > 100){
                        high = 1;
                    }
                }
            }
        }

        int sum = 0;
        for (const auto& angle : angle_sets){
//            cout << "Angle Set: " << angle.first << ":" << angle.second << endl;
            sum += angle.second;
        }
//        cout << "sum = " << sum << endl;
        if (sum < 2){
            return false;
        }
    }

//    cout << "square" << endl;
    return true;
}
#endif



}
}
}

