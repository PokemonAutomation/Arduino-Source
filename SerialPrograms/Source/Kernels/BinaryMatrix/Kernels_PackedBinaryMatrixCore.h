/*  Packed Binary Matrix Base
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_PackedBinaryMatrixCore_H
#define PokemonAutomation_Kernels_PackedBinaryMatrixCore_H

#include <stdint.h>
#include <string>
#include <iostream>
#include "Common/Compiler.h"
#include "Common/Cpp/Containers/AlignedVector.h"

namespace PokemonAutomation{
namespace Kernels{


// The 2D index of a tile inside binary matrix `PackedBinaryMatrixCore`
class TileIndex{
public:
    TileIndex(size_t x, size_t y)
        : m_index((uint32_t)x | ((uint64_t)y << 32))
    {
        if ((x | y) & 0xffffffff00000000){
            std::cerr << "Pixel Overflow: (" << x << "," << y << ")" << std::endl;
        }
    }

    PA_FORCE_INLINE size_t x() const{ return (uint32_t)m_index; }
    PA_FORCE_INLINE size_t y() const{ return (uint32_t)(m_index >> 32); }

    PA_FORCE_INLINE friend bool operator<(const TileIndex& a, const TileIndex& b){
        return a.m_index < b.m_index;
    }

private:
    uint64_t m_index;
};




template <typename TileType>
class PackedBinaryMatrixCore{
public:
    using Tile = TileType;

public:
    //  Rule of 5
    ~PackedBinaryMatrixCore();
    PackedBinaryMatrixCore(PackedBinaryMatrixCore&& x);
    void operator=(PackedBinaryMatrixCore&& x);
    PackedBinaryMatrixCore(const PackedBinaryMatrixCore& x);
    void operator=(const PackedBinaryMatrixCore& x);

public:
    //  Construction
    PackedBinaryMatrixCore();
    PackedBinaryMatrixCore(size_t width, size_t height);

    void clear();

    void set_zero();        //  Zero the entire matrix.
    void set_ones();        //  Set entire matrix to ones.
    void invert();          //  Invert all bits.

    //  Matrix must have same dimensions.
    void operator^=(const PackedBinaryMatrixCore& x);
    void operator|=(const PackedBinaryMatrixCore& x);
    void operator&=(const PackedBinaryMatrixCore& x);

public:
    // How many pixels in an image row, which is equal to how many bits in a binary matrix row 
    size_t width() const{ return m_logical_width; }
    // How many pixels in an image column, which is equal to how many bits in a binary matrix column
    size_t height() const{ return m_logical_height; }

    //  These are slow.
    bool get(size_t x, size_t y) const;
    void set(size_t x, size_t y, bool set);

    PackedBinaryMatrixCore submatrix(size_t x, size_t y, size_t width, size_t height) const;

    // Print entire binary matrix as 0s and 1s. Rows are ended with "\n".
    std::string dump() const;
    // Print part of max as 0s and 1s. Rows are ended with "\n".
    std::string dump(size_t min_x, size_t min_y, size_t max_x, size_t max_y) const;
    // Print all the underlying tiles that form this binary matrix. The result is a matrix that may be larger
    // than the original matrix.
    std::string dump_tiles() const;

public:
    // How many tiles in a row.
    size_t tile_width() const{ return m_tile_width; }
    // How many tiles in a column
    size_t tile_height() const{ return m_tile_height; }

    // Get (index.x()-th, index.y()-th) tile
    const TileType& tile(TileIndex index) const;
          TileType& tile(TileIndex index);
    // Get (x-th, y-th) tile
    const TileType& tile(size_t x, size_t y) const;
          TileType& tile(size_t x, size_t y);

public:
    //  Word Access. How many words in a row. One word is 8 bytes (aka 64 bits).
    //  For different simd implementations, every tile is always one word wide.
    size_t word64_width() const{ return m_tile_width; }
    //  Word Access. How many words in a column. One word is 8 bytes (aka 64 bits).
    size_t word64_height() const{ return m_logical_height; }

    // Get (x-th, y-th) word. One word is 8 bytes (aka 64 bits), one row in a tile.
    uint64_t word64(size_t x, size_t y) const;
    // Get (x-th, y-th) word. One word is 8 bytes (aka 64 bits), one row in a tile.
    uint64_t& word64(size_t x, size_t y);

private:
    static constexpr size_t TILE_WIDTH = TileType::WIDTH;
    static constexpr size_t TILE_HEIGHT = TileType::HEIGHT;

    // How many bits in a row (aka how many pixels in an image row)
    size_t m_logical_width;
    // How many bits in a column (aka how many pixels in an image column)
    size_t m_logical_height;
    // How many tiles in a row
    size_t m_tile_width;
    // How many tiles in a column
    size_t m_tile_height;
    AlignedVector<TileType> m_data;
};





//  Implementations



//  Tile Access

template <typename Tile> PA_FORCE_INLINE
const Tile& PackedBinaryMatrixCore<Tile>::tile(TileIndex index) const{
    return m_data[index.x() + index.y() * m_tile_width];
}
template <typename Tile> PA_FORCE_INLINE
Tile& PackedBinaryMatrixCore<Tile>::tile(TileIndex index){
    return m_data[index.x() + index.y() * m_tile_width];
}
template <typename Tile> PA_FORCE_INLINE
const Tile& PackedBinaryMatrixCore<Tile>::tile(size_t x, size_t y) const{
    return m_data[x + y * m_tile_width];
}
template <typename Tile> PA_FORCE_INLINE
Tile& PackedBinaryMatrixCore<Tile>::tile(size_t x, size_t y){
    return m_data[x + y * m_tile_width];
}



//  Word Access

template <typename Tile> PA_FORCE_INLINE
uint64_t PackedBinaryMatrixCore<Tile>::word64(size_t x, size_t y) const{
    static_assert(TILE_WIDTH == 64);
    const Tile& tile = this->tile(x, y / TILE_HEIGHT);
    return tile.row(y % TILE_HEIGHT);
}
template <typename Tile> PA_FORCE_INLINE
uint64_t& PackedBinaryMatrixCore<Tile>::word64(size_t x, size_t y){
    static_assert(TILE_WIDTH == 64);
    Tile& tile = this->tile(x, y / TILE_HEIGHT);
    return tile.row(y % TILE_HEIGHT);
}



//  Bit Access

//  Get the bit at bit location (x, y)
template <typename Tile>
bool PackedBinaryMatrixCore<Tile>::get(size_t x, size_t y) const{
    const Tile& tile = this->tile(x / TILE_WIDTH, y / TILE_HEIGHT);
    return tile.get_bit(x % TILE_WIDTH, y % TILE_HEIGHT);
}

//  Set the bit at bit location (x, y) to `set`
template <typename Tile>
void PackedBinaryMatrixCore<Tile>::set(size_t x, size_t y, bool set){
    Tile& tile = this->tile(x / TILE_WIDTH, y / TILE_HEIGHT);
    tile.set_bit(x % TILE_WIDTH, y % TILE_HEIGHT, set);
}



}
}
#endif
