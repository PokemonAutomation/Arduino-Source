/*  Packed Binary Matrix Base
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_PackedBinaryMatrixCore_TPP
#define PokemonAutomation_Kernels_PackedBinaryMatrixCore_TPP

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels_PackedBinaryMatrixCore.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{



//  Rule of 5

template <typename Tile>
PackedBinaryMatrixCore<Tile>::~PackedBinaryMatrixCore(){}
template <typename Tile>
PackedBinaryMatrixCore<Tile>::PackedBinaryMatrixCore(PackedBinaryMatrixCore&& x)
    : m_logical_width(x.m_logical_width)
    , m_logical_height(x.m_logical_height)
    , m_tile_width(x.m_tile_width)
    , m_tile_height(x.m_tile_height)
    , m_data(std::move(x.m_data))
{
    x.m_logical_width = 0;
    x.m_logical_height = 0;
    x.m_tile_width = 0;
    x.m_tile_height = 0;
}
template <typename Tile>
void PackedBinaryMatrixCore<Tile>::operator=(PackedBinaryMatrixCore&& x){
    m_logical_width = x.m_logical_width;
    m_logical_height = x.m_logical_height;
    m_tile_width = x.m_tile_width;
    m_tile_height = x.m_tile_height;
    m_data = std::move(x.m_data);
    x.m_logical_width = 0;
    x.m_logical_height = 0;
    x.m_tile_width = 0;
    x.m_tile_height = 0;
}
template <typename Tile>
PackedBinaryMatrixCore<Tile>::PackedBinaryMatrixCore(const PackedBinaryMatrixCore& x)
    : m_logical_width(x.m_logical_width)
    , m_logical_height(x.m_logical_height)
    , m_tile_width(x.m_tile_width)
    , m_tile_height(x.m_tile_height)
    , m_data(m_tile_width * m_tile_height)
{
    size_t stop = m_tile_width * m_tile_height;
    for (size_t c = 0; c < stop; c++){
        m_data[c] = x.m_data[c];
    }
}
template <typename Tile>
void PackedBinaryMatrixCore<Tile>::operator=(const PackedBinaryMatrixCore& x){
    m_logical_width = x.m_logical_width;
    m_logical_height = x.m_logical_height;
    m_tile_width = x.m_tile_width;
    m_tile_height = x.m_tile_height;
    m_data = x.m_data;
}


//  Construction

template <typename Tile>
PackedBinaryMatrixCore<Tile>::PackedBinaryMatrixCore()
    : m_logical_width(0)
    , m_logical_height(0)
    , m_tile_width(0)
    , m_tile_height(0)
{}
template <typename Tile>
PackedBinaryMatrixCore<Tile>::PackedBinaryMatrixCore(size_t width, size_t height)
    : m_logical_width(width)
    , m_logical_height(height)
    , m_tile_width((width + TILE_WIDTH - 1) / TILE_WIDTH)
    , m_tile_height((height + TILE_HEIGHT - 1) / TILE_HEIGHT)
    , m_data(m_tile_width * m_tile_height)
{}
template <typename Tile>
void PackedBinaryMatrixCore<Tile>::clear(){
    m_logical_width = 0;
    m_logical_height = 0;
    m_tile_width = 0;
    m_tile_height = 0;
    m_data.clear();
}
template <typename Tile>
void PackedBinaryMatrixCore<Tile>::set_zero(){
    size_t stop = m_tile_width * m_tile_height;
    for (size_t c = 0; c < stop; c++){
        m_data[c].set_zero();
    }
}
template <typename Tile>
void PackedBinaryMatrixCore<Tile>::set_ones(){
    //  This one is more complicated because because we need to leave the
    //  padding its zero.
    size_t r = 0;
    size_t r_left = m_logical_height;
    while (r_left >= TILE_HEIGHT){
        size_t c = 0;
        size_t c_left = m_logical_width;
        while (c_left >= TILE_WIDTH){
            this->tile(c, r).set_ones();
            c++;
            c_left -= TILE_WIDTH;
        }
        if (c_left > 0){
            this->tile(c, r).set_ones(c_left, TILE_HEIGHT);
        }
        r++;
        r_left -= TILE_HEIGHT;
    }
    if (r_left > 0){
        size_t c = 0;
        size_t c_left = m_logical_width;
        while (c_left >= TILE_WIDTH){
            this->tile(c, r).set_ones(TILE_WIDTH, r_left);
            c++;
            c_left -= TILE_WIDTH;
        }
        if (c_left > 0){
            this->tile(c, r).set_ones(c_left, r_left);
        }

    }
}
template <typename Tile>
void PackedBinaryMatrixCore<Tile>::invert(){
    //  This one is more complicated because because we need to leave the
    //  padding its zero.
    size_t r = 0;
    size_t r_left = m_logical_height;
    while (r_left >= TILE_HEIGHT){
        size_t c = 0;
        size_t c_left = m_logical_width;
        while (c_left >= TILE_WIDTH){
            this->tile(c, r).invert();
            c++;
            c_left -= TILE_WIDTH;
        }
        if (c_left > 0){
            Tile& tile = this->tile(c, r);
            tile.invert();
            tile.clear_padding(c_left, TILE_HEIGHT);
        }
        r++;
        r_left -= TILE_HEIGHT;
    }
    if (r_left > 0){
        size_t c = 0;
        size_t c_left = m_logical_width;
        while (c_left >= TILE_WIDTH){
            Tile& tile = this->tile(c, r);
            tile.invert();
            tile.clear_padding(TILE_WIDTH, r_left);
            c++;
            c_left -= TILE_WIDTH;
        }
        if (c_left > 0){
            Tile& tile = this->tile(c, r);
            tile.invert();
            tile.clear_padding(c_left, r_left);
        }

    }
}
template <typename Tile>
void PackedBinaryMatrixCore<Tile>::operator^=(const PackedBinaryMatrixCore& x){
    if (m_logical_width != x.m_logical_width || m_logical_height != x.m_logical_height){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatching dimensions.");
    }
    size_t stop = m_tile_width * m_tile_height;
    for (size_t c = 0; c < stop; c++){
        m_data[c] ^= x.m_data[c];
    }
}
template <typename Tile>
void PackedBinaryMatrixCore<Tile>::operator|=(const PackedBinaryMatrixCore& x){
    if (m_logical_width != x.m_logical_width || m_logical_height != x.m_logical_height){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatching dimensions.");
    }
    size_t stop = m_tile_width * m_tile_height;
    for (size_t c = 0; c < stop; c++){
        m_data[c] |= x.m_data[c];
    }
}
template <typename Tile>
void PackedBinaryMatrixCore<Tile>::operator&=(const PackedBinaryMatrixCore& x){
    if (m_logical_width != x.m_logical_width || m_logical_height != x.m_logical_height){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatching dimensions.");
    }
    size_t stop = m_tile_width * m_tile_height;
    for (size_t c = 0; c < stop; c++){
        m_data[c] &= x.m_data[c];
    }
}



//  Debugging

template <typename Tile>
std::string PackedBinaryMatrixCore<Tile>::dump() const{
    return dump(0, 0, m_logical_width, m_logical_height);
}
template <typename Tile>
std::string PackedBinaryMatrixCore<Tile>::dump(
    size_t min_x, size_t min_y,
    size_t max_x, size_t max_y
) const{
    std::string str;
    for (size_t r = min_y; r < max_y; r++){
        for (size_t c = min_x; c < max_x; c++){
            str += get(c, r) ? '1' : '0';
        }
        str += "\n";
    }
    return str;
}
template <typename Tile>
std::string PackedBinaryMatrixCore<Tile>::dump_tiles() const{
    return dump(0, 0, m_tile_width * TILE_WIDTH, m_tile_height * TILE_HEIGHT);
}




template <typename Tile>
PackedBinaryMatrixCore<Tile> PackedBinaryMatrixCore<Tile>::submatrix(
    size_t x, size_t y,
    size_t width, size_t height
) const{
    PackedBinaryMatrixCore ret(width, height);

    //  Completely out-of-bounds.
    if (x >= m_logical_width){
        return ret;
    }
    if (y >= m_logical_height){
        return ret;
    }

    //  Partially out-of-bounds.
    width = std::min(width, m_logical_width - x);
    height = std::min(height, m_logical_height - y);

    size_t tile_width = (width + TILE_WIDTH - 1) / TILE_WIDTH;
    size_t tile_height = (height + TILE_HEIGHT - 1) / TILE_HEIGHT;
    size_t tile_shift_x = x / TILE_WIDTH;
    size_t tile_shift_y = y / TILE_HEIGHT;
    size_t bit_shift_x = x % TILE_WIDTH;
    size_t bit_shift_y = y % TILE_HEIGHT;

//    cout << "bit_shift_x = " << bit_shift_x << endl;
//    cout << "bit_shift_y = " << bit_shift_y << endl;

    for (size_t r = 0; r < tile_height; r++){
        for (size_t c = 0; c < tile_width; c++){
            //  For tile in the destination matrix, populate it by extracting
            //  from the 4 tiles that it overlaps in the source.
            size_t src_x = tile_shift_x + c;
            size_t src_y = tile_shift_y + r;
            Tile& tile = ret.tile(c, r);

            //  Upper-left tile is always there.
            this->tile(src_x, src_y).copy_to_shift_pp(tile, bit_shift_x, bit_shift_y);

            bool shift_x = src_x + 1 < m_tile_width && bit_shift_x != 0;
            bool shift_y = src_y + 1 < m_tile_height && bit_shift_y != 0;

            //  Upper-right is there only if horizontally misaligned.
            if (shift_x){
                this->tile(src_x + 1, src_y).copy_to_shift_np(tile, TILE_WIDTH - bit_shift_x, bit_shift_y);
            }

            //  Lower-left is there only if vertically misaligned.
            if (shift_y){
                this->tile(src_x, src_y + 1).copy_to_shift_pn(tile, bit_shift_x, TILE_HEIGHT - bit_shift_y);
            }

            //  Lower-right if both horizontally and vertically misaligned.
            if (shift_x && shift_y){
                this->tile(src_x + 1, src_y + 1).copy_to_shift_nn(tile, TILE_WIDTH - bit_shift_x, TILE_HEIGHT - bit_shift_y);
            }
        }
    }

#if 1
    size_t wbits = width % TILE_WIDTH;
    for (size_t r = 0; r < tile_height; r++){
        ret.tile(tile_width - 1, r).clear_padding(wbits, TILE_HEIGHT);
    }
    size_t hbits = height % TILE_HEIGHT;
    for (size_t c = 0; c < tile_width; c++){
        ret.tile(c, tile_height - 1).clear_padding(TILE_WIDTH, hbits);
    }
#endif

    return ret;
}





}
}
#endif
