/*  Binary Matrix Base
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixBase_H
#define PokemonAutomation_Kernels_BinaryMatrixBase_H

#include <string>
#include "Common/Cpp/AlignedVector.h"

namespace PokemonAutomation{
namespace Kernels{


template <typename TileType>
class BinaryMatrixBase{
public:
    using Tile = TileType;

public:
    //  Rule of 5
    BinaryMatrixBase(BinaryMatrixBase&& x);
    void operator=(BinaryMatrixBase&& x);
    BinaryMatrixBase(const BinaryMatrixBase& x);
    void operator=(const BinaryMatrixBase& x);

public:
    //  Construction
    BinaryMatrixBase(size_t width, size_t height);

    void set_zero();        //  Zero the entire matrix.
    void set_ones();        //  Set entire matrix to ones.

public:
    size_t width() const{ return m_logical_width; }
    size_t height() const{ return m_logical_height; }

    //  These are slow.
    bool get(size_t x, size_t y) const;
    void set(size_t x, size_t y, bool set);

    BinaryMatrixBase submatrix(size_t x, size_t y, size_t width, size_t height) const;

    std::string dump() const;
    std::string dump(size_t min_x, size_t min_y, size_t max_x, size_t max_y) const;

public:
    //  Tile Access
    size_t tile_width() const{ return m_tile_width; }
    size_t tile_height() const{ return m_tile_height; }

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

template <typename Tile>
const Tile& BinaryMatrixBase<Tile>::tile(size_t x, size_t y) const{
    return m_data[x + y * m_tile_width];
}
template <typename Tile>
Tile& BinaryMatrixBase<Tile>::tile(size_t x, size_t y){
    return m_data[x + y * m_tile_width];
}



//  Word Access

template <typename Tile>
uint64_t BinaryMatrixBase<Tile>::word64(size_t x, size_t y) const{
    static_assert(TILE_WIDTH == 64);
    const Tile& tile = this->tile(x, y / TILE_HEIGHT);
    return tile.row(y % TILE_HEIGHT);
}
template <typename Tile>
uint64_t& BinaryMatrixBase<Tile>::word64(size_t x, size_t y){
    static_assert(TILE_WIDTH == 64);
    Tile& tile = this->tile(x, y / TILE_HEIGHT);
    return tile.row(y % TILE_HEIGHT);
}



//  Bit Access

template <typename Tile>
bool BinaryMatrixBase<Tile>::get(size_t x, size_t y) const{
    const Tile& tile = this->tile(x / TILE_WIDTH, y / TILE_HEIGHT);
    return tile.get_bit(x % TILE_WIDTH, y % TILE_HEIGHT);
}
template <typename Tile>
void BinaryMatrixBase<Tile>::set(size_t x, size_t y, bool set){
    Tile& tile = this->tile(x / TILE_WIDTH, y / TILE_HEIGHT);
    tile.set_bit(x % TILE_WIDTH, y % TILE_HEIGHT, set);
}



}
}
#endif
