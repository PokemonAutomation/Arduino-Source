/*  Fill Geometry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Return a set of connected clusters.
 *
 */

#include "FillGeometry.h"

namespace PokemonAutomation{


FillGeometry fill_geometry(std::vector<std::vector<MapState>>& map, int x, int y){
    int w = map[0].size();
    int h = map.size();

    int min_x = x;
    int max_x = x;
    int min_y = y;
    int max_y = y;

    size_t area = 0;
    uint64_t sum_x = 0;
    uint64_t sum_y = 0;

    std::vector<std::pair<int, int>> queue;
    queue.emplace_back(x, y);
    map[y][x] = MapState::QUEUED;

    while (!queue.empty()){
        x = queue.back().first;
        y = queue.back().second;
        queue.pop_back();
        MapState& pixel = map[y][x];
        pixel = MapState::DONE;
        area++;
        sum_x += x;
        sum_y += y;
        {
            MapState& neighbor = map[y - 1][x];
            if (y - 1 >= 0 && neighbor == MapState::EMPTY){
                neighbor = MapState::QUEUED;
                queue.emplace_back(x, y - 1);
                min_y = std::min(min_y, y - 1);
            }
        }
        {
            MapState& neighbor = map[y + 1][x];
            if (y + 1 < h && neighbor == MapState::EMPTY){
                neighbor = MapState::QUEUED;
                queue.emplace_back(x, y + 1);
                max_y = std::max(max_y, y + 1);
            }
        }
        {
            MapState& neighbor = map[y][x - 1];
            if (x - 1 >= 0 && neighbor == MapState::EMPTY){
                neighbor = MapState::QUEUED;
                queue.emplace_back(x - 1, y);
                min_x = std::min(min_x, x - 1);
            }
        }
        {
            MapState& neighbor = map[y][x + 1];
            if (x + 1 < w && neighbor == MapState::EMPTY){
                neighbor = MapState::QUEUED;
                queue.emplace_back(x + 1, y);
                max_x = std::max(max_x, x + 1);
            }
        }
    }

    return FillGeometry{
        min_x, min_y, max_x + 1, max_y + 1,
        area, (int)(sum_x / area), (int)(sum_y / area)
    };
}


bool fill_geometry(
    FillGeometry& geometry,
    FlagMatrix& matrix,
    int x, int y,
    FlagMatrix::CellType required_mask,
    FlagMatrix::CellType visited_mask
){
    FlagMatrix::CellType visited = required_mask | visited_mask;
    if ((matrix[y][x] & required_mask) != required_mask){
        return false;
    }

    int width = matrix.width();
    int height = matrix.height();

    int min_x = x;
    int max_x = x;
    int min_y = y;
    int max_y = y;

    size_t area = 0;
    uint64_t sum_x = 0;
    uint64_t sum_y = 0;


    std::vector<std::pair<int, int>> queue;
    queue.emplace_back(x, y);
    matrix[y][x] |= visited;

    while (!queue.empty()){
        x = queue.back().first;
        y = queue.back().second;
        queue.pop_back();

#if 0
        //  Out of bounds.
        if (x < 0 || y < 0 || x >= width || y >= height){
            continue;
        }
        //  Not active.
        if ((matrix[y][x] & visited) != required_mask){
            continue;
        }
#endif

        area++;
        sum_x += x;
        sum_y += y;

        //  Load the verticals.
        bool hit = false;
        if (y - 1 >= 0){
            FlagMatrix::CellType& neighbor = matrix[y - 1][x];
            if ((neighbor & visited) == required_mask){
                neighbor |= visited;
                queue.emplace_back(x, y - 1);
                min_y = std::min(min_y, y - 1);
                hit = true;
            }
        }
        if (y + 1 < height){
            FlagMatrix::CellType& neighbor = matrix[y + 1][x];
            if ((neighbor & visited) == required_mask){
                neighbor |= visited;
                queue.emplace_back(x, y + 1);
                max_y = std::max(max_y, y + 1);
                hit = true;
            }
        }
        if (x - 1 >= 0){
            FlagMatrix::CellType& neighbor = matrix[y][x - 1];
            if ((neighbor & visited) == required_mask){
                neighbor |= visited;
                queue.emplace_back(x - 1, y);
                min_x = std::min(min_x, x - 1);
                hit = true;
            }
        }
        if (x + 1 < width){
            FlagMatrix::CellType& neighbor = matrix[y][x + 1];
            if ((neighbor & visited) == required_mask){
                neighbor |= visited;
                queue.emplace_back(x + 1, y);
                max_x = std::max(max_x, x + 1);
                hit = true;
            }
        }

        if (hit){
            continue;
        }

        //  If all verticals are blank, try the diagonals.
        if (y - 1 >= 0 && x - 1 >= 0){
            FlagMatrix::CellType& neighbor = matrix[y - 1][x - 1];
            if ((neighbor & visited) == required_mask){
                neighbor |= visited;
                queue.emplace_back(x - 1, y - 1);
                min_x = std::min(min_x, x - 1);
                min_y = std::min(min_y, y - 1);
            }
        }
        if (y - 1 >= 0 && x + 1 < width){
            FlagMatrix::CellType& neighbor = matrix[y - 1][x + 1];
            if ((neighbor & visited) == required_mask){
                neighbor |= visited;
                queue.emplace_back(x + 1, y - 1);
                max_x = std::max(max_x, x + 1);
                min_y = std::min(min_y, y - 1);
            }
        }
        if (y + 1 < height && x - 1 >= 0){
            FlagMatrix::CellType& neighbor = matrix[y + 1][x - 1];
            if ((neighbor & visited) == required_mask){
                neighbor |= visited;
                queue.emplace_back(x - 1, y + 1);
                min_x = std::min(min_x, x - 1);
                max_y = std::max(max_y, y + 1);
            }
        }
        if (y + 1 < height && x + 1 < width){
            FlagMatrix::CellType& neighbor = matrix[y + 1][x + 1];
            if ((neighbor & visited) == required_mask){
                neighbor |= visited;
                queue.emplace_back(x + 1, y + 1);
                max_x = std::max(max_x, x + 1);
                max_y = std::max(max_y, y + 1);
            }
        }
    }

    geometry.min_x = min_x;
    geometry.min_y = min_y;
    geometry.max_x = max_x + 1;
    geometry.max_y = max_y + 1;
    geometry.area = area;
    geometry.center_x = (int)(sum_x / area);
    geometry.center_y = (int)(sum_y / area);
    return true;
}


std::vector<FillGeometry> find_all_objects(
    FlagMatrix& matrix,
    size_t y_start, size_t y_end,
    size_t x_start, size_t x_end,
    FlagMatrix::CellType required_mask,
    FlagMatrix::CellType visited_mask
){
    std::vector<FillGeometry> objects;
    for (size_t r = y_start; r < y_end; r++){
        for (size_t c = x_start; c < x_end; c++){
            FillGeometry region;
            if (fill_geometry(region, matrix, c, r, required_mask, visited_mask)){
                objects.emplace_back(region);
            }
        }
    }
    return objects;
}



}
