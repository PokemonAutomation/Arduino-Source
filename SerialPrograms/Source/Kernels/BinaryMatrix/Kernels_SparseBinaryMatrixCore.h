/*  Packed Binary Matrix Base
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_SparseBinaryMatrixCore_H
#define PokemonAutomation_Kernels_SparseBinaryMatrixCore_H

#include <string>
#include <map>
#include "Kernels_PackedBinaryMatrixCore.h"

namespace PokemonAutomation{
namespace Kernels{


template <typename TileType>
class SparseBinaryMatrixCore{
public:
    using Tile = TileType;

public:
    //  Rule of 5
    ~SparseBinaryMatrixCore();
    SparseBinaryMatrixCore(SparseBinaryMatrixCore&& x);
    void operator=(SparseBinaryMatrixCore&& x);
    SparseBinaryMatrixCore(const SparseBinaryMatrixCore& x);
    void operator=(const SparseBinaryMatrixCore& x);

public:
    //  Construction
    SparseBinaryMatrixCore();
    SparseBinaryMatrixCore(size_t width, size_t height);

    void clear();
    void set_data(std::map<TileIndex, TileType> data);

    void operator^=(const SparseBinaryMatrixCore& x);
    void operator|=(const SparseBinaryMatrixCore& x);
    void operator&=(const SparseBinaryMatrixCore& x);

public:
    size_t width() const{ return m_logical_width; }
    size_t height() const{ return m_logical_height; }

    //  These are slow.
    bool get(size_t x, size_t y) const;
    void set(size_t x, size_t y, bool set);

    PackedBinaryMatrixCore<TileType> submatrix(size_t x, size_t y, size_t width, size_t height) const;

    std::string dump() const;
    std::string dump(size_t min_x, size_t min_y, size_t max_x, size_t max_y) const;

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
    std::map<TileIndex, TileType> m_data;


    static const TileType& ZERO_TILE();
};






//  Implementations



//  Tile Access

template <typename Tile> PA_FORCE_INLINE
const Tile& SparseBinaryMatrixCore<Tile>::tile(TileIndex index) const{
    auto iter = m_data.find(index);
    if (iter == m_data.end()){
        return ZERO_TILE();
    }
    return iter->second;
}
template <typename Tile> PA_FORCE_INLINE
Tile& SparseBinaryMatrixCore<Tile>::tile(TileIndex index){
    return m_data[index];
}
template <typename Tile> PA_FORCE_INLINE
const Tile& SparseBinaryMatrixCore<Tile>::tile(size_t x, size_t y) const{
    return tile(TileIndex(x, y));
}
template <typename Tile> PA_FORCE_INLINE
Tile& SparseBinaryMatrixCore<Tile>::tile(size_t x, size_t y){
    return tile(TileIndex(x, y));
}



//  Word Access

template <typename Tile> PA_FORCE_INLINE
uint64_t SparseBinaryMatrixCore<Tile>::word64(size_t x, size_t y) const{
    static_assert(TILE_WIDTH == 64);
    const Tile& tile = this->tile(x, y / TILE_HEIGHT);
    return tile.row(y % TILE_HEIGHT);
}
template <typename Tile> PA_FORCE_INLINE
uint64_t& SparseBinaryMatrixCore<Tile>::word64(size_t x, size_t y){
    static_assert(TILE_WIDTH == 64);
    Tile& tile = this->tile(x, y / TILE_HEIGHT);
    return tile.row(y % TILE_HEIGHT);
}



//  Bit Access

template <typename Tile>
bool SparseBinaryMatrixCore<Tile>::get(size_t x, size_t y) const{
    const Tile& tile = this->tile(x / TILE_WIDTH, y / TILE_HEIGHT);
    return tile.get_bit(x % TILE_WIDTH, y % TILE_HEIGHT);
}
template <typename Tile>
void SparseBinaryMatrixCore<Tile>::set(size_t x, size_t y, bool set){
    Tile& tile = this->tile(x / TILE_WIDTH, y / TILE_HEIGHT);
    tile.set_bit(x % TILE_WIDTH, y % TILE_HEIGHT, set);
}




}
}
#endif
