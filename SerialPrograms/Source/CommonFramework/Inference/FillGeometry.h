/*  Fill Geometry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Return a set of connected clusters.
 *
 */

#ifndef PokemonAutomation_CommonFramework_FillGeometry_H
#define PokemonAutomation_CommonFramework_FillGeometry_H

#include <stdint.h>
#include <vector>
#include "InferenceTypes.h"
#include "FillMatrix.h"

namespace PokemonAutomation{

struct FillGeometry{
    PixelBox box;
    FillMatrix::ObjectID id = 0;
    size_t area = 0;
    int center_x = 0;
    int center_y = 0;
};

enum class MapState : uint8_t{
    WALL,
    EMPTY,
    QUEUED,
    DONE,
};


bool fill_geometry(
    FillGeometry& geometry,
    FillMatrix& matrix, FillMatrix::ObjectID required_existing_id,
    int x, int y, bool allow_diagonal,
    FillMatrix::ObjectID id
);
std::vector<FillGeometry> find_all_objects(
    FillMatrix& matrix, FillMatrix::ObjectID required_existing_id,
    bool allow_diagonal,
    size_t min_area = 10
);




}
#endif
