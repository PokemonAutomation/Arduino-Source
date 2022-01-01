/*  Binary Matrix Tile (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixTile_Default_H
#define PokemonAutomation_Kernels_BinaryMatrixTile_Default_H

#include "Common/Compiler.h"
#include "Kernels_BinaryMatrixTile_Debugging.h"

namespace PokemonAutomation{
namespace Kernels{


struct BinaryTile_Default{
    static constexpr size_t WIDTH = 64;
    static constexpr size_t HEIGHT = 4;

    uint64_t vec[4];

    PA_FORCE_INLINE BinaryTile_Default() = default;

    PA_FORCE_INLINE void set_zero(){
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = 0;
        vec[3] = 0;
    }
    uint64_t row(size_t index) const{
        return vec[index];
    }
    uint64_t& row(size_t index){
        return vec[index];
    }

    std::string dump() const{
        std::string str;
        for (size_t c = 0; c < 4; c++){
            str += dump64(row(c)) + "\n";
        }
        return str;
    }
};





}
}
#endif
