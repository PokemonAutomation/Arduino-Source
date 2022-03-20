/*  Waterfill Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Routines_H
#define PokemonAutomation_Kernels_Waterfill_Routines_H

#include <vector>
#include <set>
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_t.h"
#include "Kernels/BinaryMatrix/Kernels_PackedBinaryMatrixCore.h"
#include "Kernels/BinaryMatrix/Kernels_SparseBinaryMatrixCore.h"
#include "Kernels_Waterfill_Types.h"
#include "Kernels_Waterfill.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


template <typename Tile, typename TileRoutines>
bool find_object(
    PackedBinaryMatrixCore<Tile>& matrix,
    WaterfillObject& object,
    size_t tile_x, size_t tile_y,
    size_t bit_x, size_t bit_y
){
    std::set<TileIndex> busy;
    std::map<TileIndex, Tile> obj;

    //  Set first tile.
    busy.insert({tile_x, tile_y});
    obj[{tile_x, tile_y}].set_bit(bit_x, bit_y, 1);

    //  Iterate Waterfill...
    while (!busy.empty()){
        auto current = busy.begin();
        TileIndex index = *current;
        size_t x = index.x();
        size_t y = index.y();

        Tile& mask = matrix.tile(x, y);
        Tile& tile = obj[index];

        //  Expand current tile.
        TileRoutines::Waterfill_expand(mask, tile);
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
                if (TileRoutines::Waterfill_touch_bottom(neighbor_mask, iter->second, tile)){
                    busy.insert(neightbor_index);
                }
            }else{
                neighbor_scratch.set_zero();
                if (TileRoutines::Waterfill_touch_bottom(neighbor_mask, neighbor_scratch, tile)){
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
                if (TileRoutines::Waterfill_touch_top(neighbor_mask, iter->second, tile)){
                    busy.insert(neightbor_index);
                }
            }else{
                neighbor_scratch.set_zero();
                if (TileRoutines::Waterfill_touch_top(neighbor_mask, neighbor_scratch, tile)){
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
                if (TileRoutines::Waterfill_touch_right(neighbor_mask, iter->second, tile)){
                    busy.insert(neightbor_index);
                }
            }else{
                neighbor_scratch.set_zero();
                if (TileRoutines::Waterfill_touch_right(neighbor_mask, neighbor_scratch, tile)){
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
                if (TileRoutines::Waterfill_touch_left(neighbor_mask, iter->second, tile)){
                    busy.insert(neightbor_index);
                }
            }else{
                neighbor_scratch.set_zero();
                if (TileRoutines::Waterfill_touch_left(neighbor_mask, neighbor_scratch, tile)){
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
    WaterfillObject stats;
    stats.body_x = tile_x * Tile::WIDTH + bit_x;
    stats.body_y = tile_y * Tile::HEIGHT + bit_y;
    for (const auto& item : obj){
        size_t x = item.first.x();
        size_t y = item.first.y();
        uint64_t popcount, sum_x, sum_y;
        popcount = TileRoutines::popcount_sumcoord(sum_x, sum_y, item.second);
//        if (popcount == 7 && tile_x == 16 && tile_y == 73){
//            cout << popcount << ", " << sum_x << ", " << sum_y << endl;
//            cout << item.second.dump() << endl;
//        }
        stats.accumulate_body(
            x * Tile::WIDTH, y * Tile::HEIGHT,
            popcount, sum_x, sum_y
        );
        if (!(tile_min_x < x && x < tile_max_x && tile_min_y < y && y < tile_max_y)){
            size_t cmin_x, cmax_x, cmin_y, cmax_y;
            TileRoutines::boundaries(item.second, cmin_x, cmax_x, cmin_y, cmax_y);
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
//    object.object = SparseBinaryMatrix(matrix.width(), matrix.height());
//    object.object.set_data(std::move(obj));
//    object.object = make_SparseBinaryMatrix(Tile::TYPE, matrix.width(), matrix.height());
    auto ptr = std::make_unique<SparseBinaryMatrix_t<Tile>>(matrix.width(), matrix.height());
    ptr->get().set_data(std::move(obj));
    object.object = std::move(ptr);

    return true;
}



template <typename Tile, typename TileRoutines>
bool find_object_in_tile(
    PackedBinaryMatrixCore<Tile>& matrix,
    WaterfillObject& object,
    size_t tile_x, size_t tile_y
){
    Tile& start = matrix.tile(tile_x, tile_y);

    size_t bit_x, bit_y;
    if (!TileRoutines::find_bit(bit_x, bit_y, start)){
        return false;
    }

    return find_object<Tile, TileRoutines>(matrix, object, tile_x, tile_y, bit_x, bit_y);
}


template <typename Tile, typename TileRoutines>
bool find_object_on_bit(
    PackedBinaryMatrixCore<Tile>& matrix,
    WaterfillObject& object,
    size_t x, size_t y
){
    if (!matrix.get(x, y)){
        return false;
    }

    std::set<TileIndex> busy;
    std::map<TileIndex, Tile> obj;

    size_t tile_x = x / PackedBinaryMatrixCore<Tile>::Tile::WIDTH;
    size_t tile_y = y / PackedBinaryMatrixCore<Tile>::Tile::HEIGHT;
    size_t bit_x = x % PackedBinaryMatrixCore<Tile>::Tile::WIDTH;
    size_t bit_y = y % PackedBinaryMatrixCore<Tile>::Tile::HEIGHT;

    return find_object<Tile, TileRoutines>(
        matrix, object,
        tile_x, tile_y,
        bit_x, bit_y
    );
}



template <typename Tile, typename TileRoutines>
std::vector<WaterfillObject> find_objects_inplace(PackedBinaryMatrixCore<Tile>& matrix, size_t min_area, bool keep_objects){
    std::vector<WaterfillObject> ret;
    for (size_t r = 0; r < matrix.tile_height(); r++){
        for (size_t c = 0; c < matrix.tile_width(); c++){
            while (true){
                WaterfillObject object;
                if (!find_object_in_tile<Tile, TileRoutines>(matrix, object, c, r)){
                    break;
                }
                if (!keep_objects){
                    object.object.reset();
                }
                if (object.area >= min_area){
                    ret.emplace_back(object);
                }
            }
        }
    }
    return ret;
}






template <typename Tile, typename TileRoutines>
class WaterfillIterator_TI : public WaterfillIterator2{
public:
    WaterfillIterator_TI(PackedBinaryMatrixCore<Tile>& matrix, size_t min_area)
        : WaterfillIterator2(min_area)
        , m_matrix(matrix)
    {}
    virtual bool find_next(WaterfillObject& object) override{
        while (m_tile_row < m_matrix.tile_height()){
            while (m_tile_col < m_matrix.tile_width()){
                while (true){
                    //  Not object found. Move to next tile.
                    if (!find_object_in_tile<Tile, TileRoutines>(m_matrix, object, m_tile_col, m_tile_row)){
                        break;
                    }
                    //  Object too small. Skip it.
                    if (object.area < m_min_area){
                        continue;
                    }
                    return true;
                }
                m_tile_col++;
            }
            m_tile_col = 0;
            m_tile_row++;
        }
        return false;
    }
private:
    PackedBinaryMatrixCore<Tile>& m_matrix;
};






}
}
}
#endif
