/*  Binary Matrix Tile Debugging
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixTile_Debugging_H
#define PokemonAutomation_Kernels_BinaryMatrixTile_Debugging_H

#include <cstdint>
#include <string>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{


inline std::string dump64(uint64_t x){
    std::string str(64, 0);
    for (size_t c = 0; c < 64; c++){
        str[c] = ((x >> c) & 1) ? '1' : '0';
    }
    return str;
}

inline void print64(uint64_t x){
    cout << dump64(x) << endl;
}



}
}
#endif
