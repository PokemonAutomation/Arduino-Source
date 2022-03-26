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
#include "Kernels_Waterfill_Session_TI.h"
#include "Kernels_Waterfill.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{




template <typename Tile, typename TileRoutines>
std::vector<WaterfillObject> find_objects_inplace(PackedBinaryMatrixCore<Tile>& matrix, size_t min_area){
    std::vector<WaterfillObject> ret;
    WaterfillSession_t<Tile, TileRoutines> session(matrix);
    for (size_t r = 0; r < matrix.tile_height(); r++){
        for (size_t c = 0; c < matrix.tile_width(); c++){
            while (true){
                WaterfillObject object;
                if (!session.find_object_in_tile(object, false, c, r)){
                    break;
                }
                object.object.reset();
                if (object.area >= min_area){
                    ret.emplace_back(object);
                }
            }
        }
    }
    return ret;
}






template <typename Tile, typename TileRoutines>
class WaterfillIterator_TI final : public WaterfillIterator{
public:
    WaterfillIterator_TI(PackedBinaryMatrixCore<Tile>& matrix, size_t min_area)
        : m_min_area(min_area)
        , m_session(matrix)
    {}
    virtual bool find_next(WaterfillObject& object, bool keep_object) override{
        object.session = this;
        while (m_tile_row < m_session.tile_height()){
            while (m_tile_col < m_session.tile_width()){
                while (true){
                    //  Not object found. Move to next tile.
                    if (!m_session.find_object_in_tile(object, keep_object, m_tile_col, m_tile_row)){
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
    WaterfillSession_t<Tile, TileRoutines> m_session;
    size_t m_min_area;
    size_t m_tile_row = 0;
    size_t m_tile_col = 0;

    std::unique_ptr<SparseBinaryMatrix_IB> m_object;
};






}
}
}
#endif
