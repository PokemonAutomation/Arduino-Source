/*  Cell Matrix
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_CellMatrix_H
#define PokemonAutomation_CommonFramework_CellMatrix_H

#include <stdint.h>
#include <vector>
#include <QImage>
#include "ImageBoxes.h"

namespace PokemonAutomation{

class CellMatrix{
public:
    using ObjectID = uint32_t;

public:
    CellMatrix(CellMatrix&& x);
    CellMatrix(const CellMatrix& x) = default;
    CellMatrix& operator=(CellMatrix&& x);
    CellMatrix& operator=(const CellMatrix& x) = default;

public:
    CellMatrix();
    CellMatrix(pxint_t width, pxint_t height);
    CellMatrix(const QImage& image);

    pxint_t width () const{ return m_width; }
    pxint_t height() const{ return m_height; }
    const ObjectID* operator[](pxint_t row) const{ return m_cells.data() + row * m_width; }
          ObjectID* operator[](pxint_t row)      { return m_cells.data() + row * m_width; }

    CellMatrix extract(ImagePixelBox box) const;
    CellMatrix extract(ImagePixelBox box, ObjectID id) const;

    std::string dump() const;

public:
    template <typename Image, typename Filter>
    bool apply_filter(Image&& image, Filter& filter);

    template <typename Image, typename Filter>
    bool apply_filter(
        Image&& image, Filter& filter,
        pxint_t y_start, pxint_t y_end,
        pxint_t x_start, pxint_t x_end
    );


private:
    pxint_t m_width;
    pxint_t m_height;
    std::vector<ObjectID> m_cells;
};




//  Templates

template <typename Image, typename Filter>
bool CellMatrix::apply_filter(Image&& image, Filter& filter){
    return apply_filter(image, filter, 0, m_height, 0, m_width);
}


template <typename Image, typename Filter>
bool CellMatrix::apply_filter(
    Image&& image, Filter& filter,
    pxint_t y_start, pxint_t y_end,
    pxint_t x_start, pxint_t x_end
){
    if (image.isNull() || image.width() != m_width || image.height() != m_height){
        return false;
    }

    ObjectID* data = m_cells.data();
    for (pxint_t r = y_start; r < y_end; r++){
        for (pxint_t c = x_start; c < x_end; c++){
            filter(data[c], image, c, r);
        }
        data += m_width;
    }

    return true;
}













}
#endif
