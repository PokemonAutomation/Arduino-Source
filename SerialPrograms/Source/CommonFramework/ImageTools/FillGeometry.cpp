/*  Fill Geometry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Return a set of connected clusters.
 *
 */

#include "FillGeometry.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{




double FillGeometry::aspect_ratio() const{
    return (double)box.width() / box.height();
}
double FillGeometry::area_ratio() const{
    return (double)area / box.area();
}
pxint_t FillGeometry::center_x() const{
    return (pxint_t)(sum_x / area);
}
pxint_t FillGeometry::center_y() const{
    return (pxint_t)(sum_y / area);
}

bool FillGeometry::assert_aspect_ratio(double lower_bound, double upper_bound) const{
    double ratio = aspect_ratio();
    return lower_bound <= ratio && ratio <= upper_bound;
}
bool FillGeometry::assert_area_ratio(double lower_bound, double upper_bound) const{
    double ratio = area_ratio();
    return lower_bound <= ratio && ratio <= upper_bound;
}

void FillGeometry::merge_assume_no_overlap(const FillGeometry& obj){
    if (obj.area == 0){
        return;
    }
    if (area == 0){
        *this = obj;
    }
    box.min_x = std::min(box.min_x, obj.box.min_x);
    box.min_y = std::min(box.min_y, obj.box.min_y);
    box.max_x = std::max(box.max_x, obj.box.max_x);
    box.max_y = std::max(box.max_y, obj.box.max_y);
    area += obj.area;
    sum_x += obj.sum_x;
    sum_y += obj.sum_y;
}





bool fill_geometry(
    FillGeometry& geometry,
    CellMatrix& matrix, CellMatrix::ObjectID required_existing_id,
    pxint_t x, pxint_t y, bool allow_diagonal,
    CellMatrix::ObjectID object_id
){
    if (matrix[y][x] != required_existing_id){
        return false;
    }

    pxint_t width = matrix.width();
    pxint_t height = matrix.height();

    pxint_t min_x = x;
    pxint_t max_x = x;
    pxint_t min_y = y;
    pxint_t max_y = y;

    size_t area = 0;
    uint64_t sum_x = 0;
    uint64_t sum_y = 0;

//    int startx = x;
//    int starty = y;

    std::vector<std::pair<pxint_t, pxint_t>> queue;
    queue.emplace_back(x, y);
    matrix[y][x] = object_id;
//    cout << "  (|" << starty << "," << startx << " : " << id << " = " << matrix[starty][startx] << ")" << endl;

    while (!queue.empty()){
        x = queue.back().first;
        y = queue.back().second;
        queue.pop_back();

        area++;
        sum_x += x;
        sum_y += y;

        //  Load the verticals.
        if (y > 0){
            CellMatrix::ObjectID& neighbor = matrix[y - 1][x];
            if (neighbor == required_existing_id){
                neighbor = object_id;
                queue.emplace_back(x, y - 1);
                min_y = std::min(min_y, y - 1);
            }
        }
        if (y + 1 < height){
            CellMatrix::ObjectID& neighbor = matrix[y + 1][x];
            if (neighbor == required_existing_id){
                neighbor = object_id;
                queue.emplace_back(x, y + 1);
                max_y = std::max(max_y, y + 1);
            }
        }
        if (x > 0){
            CellMatrix::ObjectID& neighbor = matrix[y][x - 1];
            if (neighbor == required_existing_id){
                neighbor = object_id;
                queue.emplace_back(x - 1, y);
                min_x = std::min(min_x, x - 1);
            }
        }
        if (x + 1 < width){
            CellMatrix::ObjectID& neighbor = matrix[y][x + 1];
            if (neighbor == required_existing_id){
                neighbor = object_id;
                queue.emplace_back(x + 1, y);
                max_x = std::max(max_x, x + 1);
            }
        }

        if (!allow_diagonal){
            continue;
        }

        //  If all verticals are blank, try the diagonals.
        if (y > 0 && x > 0){
            CellMatrix::ObjectID& neighbor = matrix[y - 1][x - 1];
            if (neighbor == required_existing_id){
                neighbor = object_id;
                queue.emplace_back(x - 1, y - 1);
                min_x = std::min(min_x, x - 1);
                min_y = std::min(min_y, y - 1);
            }
        }
        if (y > 0 && x + 1 < width){
            CellMatrix::ObjectID& neighbor = matrix[y - 1][x + 1];
            if (neighbor == required_existing_id){
                neighbor = object_id;
                queue.emplace_back(x + 1, y - 1);
                max_x = std::max(max_x, x + 1);
                min_y = std::min(min_y, y - 1);
            }
        }
        if (y + 1 < height && x > 0){
            CellMatrix::ObjectID& neighbor = matrix[y + 1][x - 1];
            if (neighbor == required_existing_id){
                neighbor = object_id;
                queue.emplace_back(x - 1, y + 1);
                min_x = std::min(min_x, x - 1);
                max_y = std::max(max_y, y + 1);
            }
        }
        if (y + 1 < height && x + 1 < width){
            CellMatrix::ObjectID& neighbor = matrix[y + 1][x + 1];
            if (neighbor == required_existing_id){
                neighbor = object_id;
                queue.emplace_back(x + 1, y + 1);
                max_x = std::max(max_x, x + 1);
                max_y = std::max(max_y, y + 1);
            }
        }
    }
//    cout << "  (|" << starty << "," << startx << " : " << id << " = " << matrix[starty][startx] << ")" << endl;

    geometry.box.min_x = min_x;
    geometry.box.min_y = min_y;
    geometry.box.max_x = max_x + 1;
    geometry.box.max_y = max_y + 1;
    geometry.id = object_id;
    geometry.area = area;
    geometry.sum_x = sum_x;
    geometry.sum_y = sum_y;
//    geometry.center_x = (pxint_t)(sum_x / area);
//    geometry.center_y = (pxint_t)(sum_y / area);
    return true;
}

std::vector<FillGeometry> find_all_objects(
    CellMatrix& matrix, CellMatrix::ObjectID required_existing_id,
    bool allow_diagonal,
    size_t min_area
){
    pxint_t width = matrix.width();
    pxint_t height = matrix.height();
    std::vector<FillGeometry> objects;
    CellMatrix::ObjectID id = 2;
    for (pxint_t r = 0; r < height; r++){
        for (pxint_t c = 0; c < width; c++){
            FillGeometry region;
            if (fill_geometry(region, matrix, required_existing_id, c, r, allow_diagonal, id)){
                id++;
                if (region.area >= min_area){
                    objects.emplace_back(region);
                }
            }
        }
    }
//    cout << "objects = " << objects.size() << endl;
    return objects;
}





}
