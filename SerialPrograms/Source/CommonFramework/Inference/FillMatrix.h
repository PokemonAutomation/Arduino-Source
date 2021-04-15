/*  Fill Matrix
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_FillMatrix_H
#define PokemonAutomation_CommonFramework_FillMatrix_H

#include <stdint.h>
#include <QImage>
#include "InferenceTypes.h"

namespace PokemonAutomation{

class FillMatrix{
public:
    using ObjectID = uint32_t;

public:
    FillMatrix(FillMatrix&& x);
    FillMatrix(const FillMatrix& x) = default;
    FillMatrix& operator=(FillMatrix&& x);
    FillMatrix& operator=(const FillMatrix& x) = default;

public:
    FillMatrix();
    FillMatrix(pxint_t width, pxint_t height);
    FillMatrix(const QImage& image);

    pxint_t width () const{ return m_width; }
    pxint_t height() const{ return m_height; }
    const ObjectID* operator[](pxint_t row) const{ return m_cells.data() + row * m_width; }
          ObjectID* operator[](pxint_t row)      { return m_cells.data() + row * m_width; }

    FillMatrix extract(PixelBox box) const;
    FillMatrix extract(PixelBox box, ObjectID id) const;

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
bool FillMatrix::apply_filter(Image&& image, Filter& filter){
    return apply_filter(image, filter, 0, m_height, 0, m_width);
}


template <typename Image, typename Filter>
bool FillMatrix::apply_filter(
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
