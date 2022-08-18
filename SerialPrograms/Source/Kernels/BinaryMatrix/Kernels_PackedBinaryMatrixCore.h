/*  Packed Binary Matrix Base
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
    size_t width() const{ return m_logical_width; }
    size_t height() const{ return m_logical_height; }

    //  These are slow.
    bool get(size_t x, size_t y) const;
    void set(size_t x, size_t y, bool set);

    PackedBinaryMatrixCore submatrix(size_t x, size_t y, size_t width, size_t height) const;

    std::string dump() const;
    std::string dump(size_t min_x, size_t min_y, size_t max_x, size_t max_y) const;
    std::string dump_tiles() const;

public:
    //  Tile Access
    size_t tile_width() const{ return m_tile_width; }
    size_t tile_height() const{ return m_tile_height; }

    const TileType& tile(TileIndex index) const;
          TileType& tile(TileIndex index);
    const TileType& tile(size_t x, size_t y) const;
          TileType& tile(size_t x, size_t y);

public:
    //  Word Access
    size_t word64_width() const{ return m_tile_width; }
    size_t word64_height() const{ return m_logical_height; }

    uint64_t word64(size_t x, size_t y) const;
    uint64_t& word64(size_t x, size_t y);

private:
    static constexpr size_t TILE_WIDTH = TileType::WIDTH;
    static constexpr size_t TILE_HEIGHT = TileType::HEIGHT;

    size_t m_logical_width;
    size_t m_logical_height;
    size_t m_tile_width;
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

template <typename Tile>
bool PackedBinaryMatrixCore<Tile>::get(size_t x, size_t y) const{
    const Tile& tile = this->tile(x / TILE_WIDTH, y / TILE_HEIGHT);
    return tile.get_bit(x % TILE_WIDTH, y % TILE_HEIGHT);
}
template <typename Tile>
void PackedBinaryMatrixCore<Tile>::set(size_t x, size_t y, bool set){
    Tile& tile = this->tile(x / TILE_WIDTH, y / TILE_HEIGHT);
    tile.set_bit(x % TILE_WIDTH, y % TILE_HEIGHT, set);
}



}
}
#endif
