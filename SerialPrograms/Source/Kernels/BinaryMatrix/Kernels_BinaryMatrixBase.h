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


template <typename Tile>
class BinaryMatrixBase{
public:
    //  Rule of 5
    BinaryMatrixBase(BinaryMatrixBase&& x);
    void operator=(BinaryMatrixBase&& x);
    BinaryMatrixBase(const BinaryMatrixBase& x);
    void operator=(const BinaryMatrixBase& x);

public:
    //  Basic Usage
    BinaryMatrixBase(size_t width, size_t height);

    void set_zero();
    void set_ones();

    size_t width() const{ return m_logical_width; }
    size_t height() const{ return m_logical_height; }

    //  These are slow.
    bool get(size_t x, size_t y) const;
    void set(size_t x, size_t y, bool set);

    std::string dump() const;

public:
    //  Tile Access
    size_t tile_width() const{ return m_tile_width; }
    size_t tile_height() const{ return m_tile_height; }

    uint64_t word64(size_t x, size_t y) const;
    uint64_t& word64(size_t x, size_t y);

    const Tile& tile(size_t x, size_t y) const;
    Tile& tile(size_t x, size_t y);

private:
    static constexpr size_t TILE_WIDTH = Tile::WIDTH;
    static constexpr size_t TILE_HEIGHT = Tile::HEIGHT;

    size_t m_logical_width;
    size_t m_logical_height;
    size_t m_tile_width;
    size_t m_tile_height;
    AlignedVector<Tile> m_data;
};





//  Implementations


template <typename Tile>
BinaryMatrixBase<Tile>::BinaryMatrixBase(BinaryMatrixBase&& x)
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
void BinaryMatrixBase<Tile>::operator=(BinaryMatrixBase&& x){
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
BinaryMatrixBase<Tile>::BinaryMatrixBase(const BinaryMatrixBase& x)
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
void BinaryMatrixBase<Tile>::operator=(const BinaryMatrixBase& x){
    m_logical_width = x.m_logical_width;
    m_logical_height = x.m_logical_height;
    m_tile_width = x.m_tile_width;
    m_tile_height = x.m_tile_height;
    m_data = x.m_data;
    size_t stop = m_tile_width * m_tile_height;
    for (size_t c = 0; c < stop; c++){
        m_data[c] = x.m_data[c];
    }
}

template <typename Tile>
BinaryMatrixBase<Tile>::BinaryMatrixBase(size_t width, size_t height)
    : m_logical_width(width)
    , m_logical_height(height)
    , m_tile_width((width + TILE_WIDTH - 1) / TILE_WIDTH)
    , m_tile_height((height + TILE_HEIGHT - 1) / TILE_HEIGHT)
    , m_data(m_tile_width * m_tile_height)
{
    set_zero();
}

template <typename Tile>
const Tile& BinaryMatrixBase<Tile>::tile(size_t x, size_t y) const{
    return m_data[x + y * m_tile_width];
}
template <typename Tile>
Tile& BinaryMatrixBase<Tile>::tile(size_t x, size_t y){
    return m_data[x + y * m_tile_width];
}
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
template <typename Tile>
std::string BinaryMatrixBase<Tile>::dump() const{
    std::string str;
    for (size_t r = 0; r < m_logical_height; r++){
        for (size_t c = 0; c < m_logical_width; c++){
            str += get(c, r) ? '1' : '0';
        }
        str += "\n";
    }
    return str;
}

template <typename Tile>
void BinaryMatrixBase<Tile>::set_zero(){
    size_t stop = m_tile_width * m_tile_height;
    for (size_t c = 0; c < stop; c++){
        m_data[c].set_zero();
    }
}
template <typename Tile>
void BinaryMatrixBase<Tile>::set_ones(){
    size_t r = 0;
    size_t r_left = m_logical_height;
    while (r_left >= TILE_HEIGHT){
        size_t c = 0;
        size_t c_left = m_logical_width;
        while (c_left >= TILE_WIDTH){
            tile(c, r).set_ones();
            c++;
            c_left -= TILE_WIDTH;
        }
        if (c_left > 0){
            tile(c, r).set_ones(c_left, TILE_HEIGHT);
        }
        r++;
        r_left -= TILE_HEIGHT;
    }
    if (r_left > 0){
        size_t c = 0;
        size_t c_left = m_logical_width;
        while (c_left >= TILE_WIDTH){
            tile(c, r).set_ones(TILE_WIDTH, r_left);
            c++;
            c_left -= TILE_WIDTH;
        }
        if (c_left > 0){
            tile(c, r).set_ones(c_left, r_left);
        }

    }
}


}
}
#endif
