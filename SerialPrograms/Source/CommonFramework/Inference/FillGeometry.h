/*  Fill Geometry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Return a set of connected clusters.
 *
 */

#ifndef PokemonAutomation_CommonFramework_BlackScreenDetector_H
#define PokemonAutomation_CommonFramework_BlackScreenDetector_H

#include <stdint.h>
#include <vector>
#include "FlagMatrix.h"

namespace PokemonAutomation{

struct FillGeometry{
    int min_x;
    int min_y;
    int max_x;
    int max_y;
    size_t area;
    int center_x;
    int center_y;
};

enum class MapState : uint8_t{
    WALL,
    EMPTY,
    QUEUED,
    DONE,
};

FillGeometry fill_geometry(std::vector<std::vector<MapState>>& map, int x, int y);


bool fill_geometry(
    FillGeometry& geometry,
    FlagMatrix& matrix,
    int x, int y,
    FlagMatrix::CellType required_mask,
    FlagMatrix::CellType visited_mask
);
std::vector<FillGeometry> find_all_objects(
    FlagMatrix& matrix,
    size_t y_start, size_t y_end,
    size_t x_start, size_t x_end,
    FlagMatrix::CellType required_mask,
    FlagMatrix::CellType visited_mask
);


}
#endif
