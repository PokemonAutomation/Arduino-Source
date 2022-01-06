/*  Waterfill Algorithm
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <map>
#include "Kernels_Waterfill.h"

#if 0
#elif defined PA_Arch_x64_AVX512
#include "Kernels_Waterfill_Tile_x64_AVX512.h"
#elif defined PA_Arch_x64_AVX2
#include "Kernels_Waterfill_Tile_x64_AVX2.h"
#elif defined PA_Arch_x64_SSE42
#include "Kernels_Waterfill_Tile_x64_SSE42.h"
#else
#include "Kernels_Waterfill_Tile_Default.h"
#endif

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


bool find_object(
    PackedBinaryMatrix& matrix,
    WaterFillObject& object,
    size_t tile_x, size_t tile_y
){
    using Tile = PackedBinaryMatrix::Tile;
    Tile& start = matrix.tile(tile_x, tile_y);

    size_t bit_x, bit_y;
    if (!find_bit(bit_x, bit_y, start)){
        return false;
    }

    std::set<TileIndex> busy;
    std::map<TileIndex, Tile> obj;

    //  Set first tile.
    busy.insert({tile_x, tile_y});
    obj[{tile_x, tile_y}].set_bit(bit_x, bit_y, 1);

    //  Iterate waterfill...
    while (!busy.empty()){
        auto current = busy.begin();
        TileIndex index = *current;
        size_t x = index.x();
        size_t y = index.y();

        Tile& mask = matrix.tile(x, y);
        Tile& tile = obj[index];

        //  Expand current tile.
        waterfill_expand(mask, tile);
        mask.andnot(tile);
        busy.erase(current);

//        cout << tile.dump() << endl;
//        cout << mask.dump() << endl;
//        break;

        Tile neighbor_scratch;
        if (y > 0){
            const Tile& neighbor_mask = matrix.tile(x, y - 1);
            TileIndex neightbor_index(x, y - 1);
            auto iter = obj.find(neightbor_index);
            if (iter != obj.end()){
                if (waterfill_touch_bottom(neighbor_mask, iter->second, tile)){
                    busy.insert(neightbor_index);
                }
            }else{
                neighbor_scratch.set_zero();
                if (waterfill_touch_bottom(neighbor_mask, neighbor_scratch, tile)){
                    busy.insert(neightbor_index);
                    obj[neightbor_index] = neighbor_scratch;
                }
            }
        }
        if (y + 1 < matrix.tile_height()){
            const Tile& neighbor_mask = matrix.tile(x, y + 1);
            TileIndex neightbor_index(x, y + 1);
            auto iter = obj.find(neightbor_index);
            if (iter != obj.end()){
                if (waterfill_touch_top(neighbor_mask, iter->second, tile)){
                    busy.insert(neightbor_index);
                }
            }else{
                neighbor_scratch.set_zero();
                if (waterfill_touch_top(neighbor_mask, neighbor_scratch, tile)){
                    busy.insert(neightbor_index);
                    obj[neightbor_index] = neighbor_scratch;
                }
            }
        }
        if (x > 0){
            const Tile& neighbor_mask = matrix.tile(x - 1, y);
            TileIndex neightbor_index(x - 1, y);
            auto iter = obj.find(neightbor_index);
            if (iter != obj.end()){
                if (waterfill_touch_right(neighbor_mask, iter->second, tile)){
                    busy.insert(neightbor_index);
                }
            }else{
                neighbor_scratch.set_zero();
                if (waterfill_touch_right(neighbor_mask, neighbor_scratch, tile)){
                    busy.insert(neightbor_index);
                    obj[neightbor_index] = neighbor_scratch;
                }
            }
        }
        if (x + 1 < matrix.tile_width()){
            const Tile& neighbor_mask = matrix.tile(x + 1, y);
            TileIndex neightbor_index(x + 1, y);
            auto iter = obj.find(neightbor_index);
            if (iter != obj.end()){
                if (waterfill_touch_left(neighbor_mask, iter->second, tile)){
                    busy.insert(neightbor_index);
                }
            }else{
                neighbor_scratch.set_zero();
                if (waterfill_touch_left(neighbor_mask, neighbor_scratch, tile)){
                    busy.insert(neightbor_index);
                    obj[neightbor_index] = neighbor_scratch;
                }
            }
        }

//        return true;
    }

//    for (const auto& item : obj){
//        cout << "(" << item.first.x() << "," << item.first.y() << ")" << endl;
//        cout << item.second.dump() << endl;
//    }

    //  Compute stats.
    size_t tile_min_x = (size_t)0 - 1;
    size_t tile_min_y = (size_t)0 - 1;
    size_t tile_max_x = 0;
    size_t tile_max_y = 0;
    WaterFillObject stats;
    stats.body_x = tile_x * Tile::WIDTH + bit_x;
    stats.body_y = tile_y * Tile::HEIGHT + bit_y;
    for (const auto& item : obj){
        size_t x = item.first.x();
        size_t y = item.first.y();
        uint64_t popcount, sum_x, sum_y;
        popcount = popcount_sumcoord(sum_x, sum_y, item.second);
//        cout << popcount << ", " << sum_x << ", " << sum_y << endl;
        stats.accumulate_body(
            x * Tile::WIDTH, y * Tile::HEIGHT,
            popcount, sum_x, sum_y
        );
        if (!(tile_min_x < x && x < tile_max_x && tile_min_y < y && y < tile_max_y)){
            size_t cmin_x, cmax_x, cmin_y, cmax_y;
            boundaries(item.second, cmin_x, cmax_x, cmin_y, cmax_y);
            stats.accumulate_boundary(
                x * Tile::WIDTH, y * Tile::HEIGHT,
                cmin_x, cmax_x, cmin_y, cmax_y
            );
        }
        tile_min_x = std::min(tile_min_x, x);
        tile_max_x = std::max(tile_max_x, x);
        tile_min_y = std::min(tile_min_y, y);
        tile_max_y = std::max(tile_max_y, y);
    }

#if 0
    cout << "x = (" << stats.m_min_x << "," << stats.m_max_x << ")" << endl;
    cout << "y = (" << stats.m_min_y << "," << stats.m_max_y << ")" << endl;
    cout << "area = " << stats.m_area << endl;
    cout << "sum x = " << stats.m_sum_x << endl;
    cout << "sum y = " << stats.m_sum_y << endl;
#endif

    object = stats;
    object.object = std::move(obj);

    return true;
}


std::vector<WaterFillObject> find_objects_inplace(PackedBinaryMatrix& matrix, size_t min_area, bool keep_objects){
    std::vector<WaterFillObject> ret;
    for (size_t r = 0; r < matrix.tile_height(); r++){
        for (size_t c = 0; c < matrix.tile_width(); c++){
            while (true){
                WaterFillObject object;
                if (!find_object(matrix, object, c, r)){
                    break;
                }
                if (!keep_objects){
                    object.object.clear();
                }
                if (object.area >= min_area){
                    ret.emplace_back(object);
                }
            }
        }
    }
    return ret;
}
std::vector<WaterFillObject> find_objects(const PackedBinaryMatrix& matrix, size_t min_area, bool keep_objects){
    PackedBinaryMatrix m(matrix);
    return find_objects_inplace(m, min_area, keep_objects);
}






}
}
}
