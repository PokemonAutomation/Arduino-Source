/*  Binary Matrix Tile (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixTile_64x4_Default_H
#define PokemonAutomation_Kernels_BinaryMatrixTile_64x4_Default_H

#include "Common/Compiler.h"
#include "Kernels_BinaryMatrixTile_Debugging.h"
#include "Kernels_BinaryMatrix.h"

namespace PokemonAutomation{
namespace Kernels{


struct BinaryTile_64x4_Default{
    static constexpr BinaryMatrixType TYPE = BinaryMatrixType::i64x4_Default;
    static constexpr size_t WIDTH = 64;
    static constexpr size_t HEIGHT = 4;

    uint64_t vec[4];


public:
    PA_FORCE_INLINE BinaryTile_64x4_Default(){
        set_zero();
    }


public:
    PA_FORCE_INLINE void set_zero(){
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = 0;
        vec[3] = 0;
    }
    PA_FORCE_INLINE void set_ones(){
        vec[0] = 0xffffffffffffffff;
        vec[1] = 0xffffffffffffffff;
        vec[2] = 0xffffffffffffffff;
        vec[3] = 0xffffffffffffffff;
    }
    PA_FORCE_INLINE void set_ones(size_t width, size_t height){
        uint64_t word = width < 64
            ? ((uint64_t)1 << width) - 1
            : 0xffffffffffffffff;
        size_t c = 0;
        for (; c < height; c++){
            vec[c] = word;
        }
        for (; c < 4; c++){
            vec[c] = 0;
        }
    }
    PA_FORCE_INLINE void clear_padding(size_t width, size_t height){
        uint64_t word = width < 64
            ? ((uint64_t)1 << width) - 1
            : 0xffffffffffffffff;
        size_t c = 0;
        for (; c < height; c++){
            vec[c] &= word;
        }
        for (; c < 4; c++){
            vec[c] = 0;
        }
    }
    PA_FORCE_INLINE void invert(){
        vec[0] = ~vec[0];
        vec[1] = ~vec[1];
        vec[2] = ~vec[2];
        vec[3] = ~vec[3];
    }
    PA_FORCE_INLINE void operator^=(const BinaryTile_64x4_Default& x){
        vec[0] ^= x.vec[0];
        vec[1] ^= x.vec[1];
        vec[2] ^= x.vec[2];
        vec[3] ^= x.vec[3];
    }
    PA_FORCE_INLINE void operator|=(const BinaryTile_64x4_Default& x){
        vec[0] |= x.vec[0];
        vec[1] |= x.vec[1];
        vec[2] |= x.vec[2];
        vec[3] |= x.vec[3];
    }
    PA_FORCE_INLINE void operator&=(const BinaryTile_64x4_Default& x){
        vec[0] &= x.vec[0];
        vec[1] &= x.vec[1];
        vec[2] &= x.vec[2];
        vec[3] &= x.vec[3];
    }
    PA_FORCE_INLINE void andnot(const BinaryTile_64x4_Default& x){
        vec[0] &= ~x.vec[0];
        vec[1] &= ~x.vec[1];
        vec[2] &= ~x.vec[2];
        vec[3] &= ~x.vec[3];
    }


public:
    PA_FORCE_INLINE uint64_t top() const{
        return vec[0];
    }
    PA_FORCE_INLINE uint64_t& top(){
        return vec[0];
    }
    PA_FORCE_INLINE uint64_t bottom() const{
        return vec[3];
    }
    PA_FORCE_INLINE uint64_t& bottom(){
        return vec[3];
    }

    PA_FORCE_INLINE uint64_t row(size_t index) const{
        return vec[index];
    }
    PA_FORCE_INLINE uint64_t& row(size_t index){
        return vec[index];
    }

    //  These are slow.
    bool get_bit(size_t x, size_t y) const{
        return (row(y) >> x) & 1;
    }
    void set_bit(size_t x, size_t y){
        row(y) |= (uint64_t)1 << x;
    }
    void set_bit(size_t x, size_t y, bool set){
        uint64_t bit = (uint64_t)(set ? 1 : 0) << x;
        uint64_t mask = (uint64_t)1 << x;
        uint64_t& word = row(y);
        word = (word & ~mask) | bit;
    }

    std::string dump() const{
        std::string str;
        for (size_t c = 0; c < 4; c++){
            str += dump64(row(c)) + "\n";
        }
        return str;
    }


public:
    //  Copy the current tile into "tile" while applying the specified shifts.
    //  These are used to implement submatrix extraction where the desired
    //  sub-matrix may of arbitrary shift and alignment.

    void copy_to_shift_pp(BinaryTile_64x4_Default& tile, size_t shift_x, size_t shift_y) const{
        //  (+x, +y)
        for (size_t c = 0; shift_y < 4; shift_y++, c++){
            tile.vec[c] |= vec[shift_y] >> shift_x;
        }
    }
    void copy_to_shift_np(BinaryTile_64x4_Default& tile, size_t shift_x, size_t shift_y) const{
        //  (-x, +y)
        for (size_t c = 0; shift_y < 4; shift_y++, c++){
            tile.vec[c] |= vec[shift_y] << shift_x;
        }
    }
    void copy_to_shift_pn(BinaryTile_64x4_Default& tile, size_t shift_x, size_t shift_y) const{
        //  (+x, -y)
        for (size_t c = 0; shift_y < 4; shift_y++, c++){
            tile.vec[shift_y] |= vec[c] >> shift_x;
        }
    }
    void copy_to_shift_nn(BinaryTile_64x4_Default& tile, size_t shift_x, size_t shift_y) const{
        //  (-x, -y)
        for (size_t c = 0; shift_y < 4; shift_y++, c++){
            tile.vec[shift_y] |= vec[c] << shift_x;
        }
    }

};





}
}
#endif
