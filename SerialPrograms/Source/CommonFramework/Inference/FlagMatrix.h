/*  Flag Matrix
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_FlagMatrix_H
#define PokemonAutomation_CommonFramework_FlagMatrix_H

#include <stdint.h>
#include <QImage>

namespace PokemonAutomation{

class FlagMatrix{
public:
    using CellType = uint8_t;

public:
    FlagMatrix(FlagMatrix&& x);
    FlagMatrix(const FlagMatrix& x) = default;
    FlagMatrix& operator=(FlagMatrix&& x);
    FlagMatrix& operator=(const FlagMatrix& x) = default;

public:
    FlagMatrix(size_t width, size_t height);
    FlagMatrix(const QImage& image);

    size_t width () const{ return m_width; }
    size_t height() const{ return m_height; }
    const CellType* operator[](size_t row) const{ return m_cells.data() + row * m_width; }
          CellType* operator[](size_t row)      { return m_cells.data() + row * m_width; }

public:
    template <typename Image, typename Filter>
    bool apply_filter(Image&& image, Filter& filter);

    template <typename Image, typename Filter>
    bool apply_filter(
        Image&& image, Filter& filter,
        size_t y_start, size_t y_end,
        size_t x_start, size_t x_end
    );


private:
    size_t m_width;
    size_t m_height;
    std::vector<CellType> m_cells;
};




//  Templates

template <typename Image, typename Filter>
bool FlagMatrix::apply_filter(Image&& image, Filter& filter){
    return apply_filter(image, filter, 0, m_height, 0, m_width);
}


template <typename Image, typename Filter>
bool FlagMatrix::apply_filter(
    Image&& image, Filter& filter,
    size_t y_start, size_t y_end,
    size_t x_start, size_t x_end
){
    if (image.isNull() || (size_t)image.width() != m_width || (size_t)image.height() != m_height){
        return false;
    }

    CellType* data = m_cells.data();
    for (size_t r = y_start; r < y_end; r++){
        for (size_t c = x_start; c < x_end; c++){
            filter(data[c], image, c, r);
        }
        data += m_width;
    }

    return true;
}





}
#endif
