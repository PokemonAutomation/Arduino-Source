/*  Flag Matrix
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CellMatrix.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


CellMatrix::CellMatrix(CellMatrix&& x)
    : m_width(x.m_width)
    , m_height(x.m_height)
    , m_cells(std::move(x.m_cells))
{
    x.m_width = 0;
    x.m_height = 0;
}
CellMatrix& CellMatrix::operator=(CellMatrix&& x){
    m_width = x.m_width;
    m_height = x.m_height;
    m_cells = std::move(x.m_cells);
    x.m_width = 0;
    x.m_height = 0;
    return *this;
}

CellMatrix::CellMatrix()
    : m_width(0)
    , m_height(0)
{}
CellMatrix::CellMatrix(pxint_t width, pxint_t height)
    : m_width(width)
    , m_height(height)
    , m_cells(width * height)
{}
CellMatrix::CellMatrix(const QImage& image)
    : CellMatrix(
        image.isNull() ? 0 : image.width(),
        image.isNull() ? 0 : image.height()
    )
{}

CellMatrix CellMatrix::extract(ImagePixelBox box) const{
    pxint_t width = box.width();
    pxint_t height = box.height();
    CellMatrix matrix(width, height);
    for (pxint_t r = 0; r < height; r++){
        for (pxint_t c = 0; c < width; c++){
            matrix[r][c] = (*this)[r + box.min_y][c + box.min_x];
        }
    }
    return matrix;
}
CellMatrix CellMatrix::extract(ImagePixelBox box, ObjectID id) const{
    pxint_t width = box.width();
    pxint_t height = box.height();
    CellMatrix matrix(width, height);
    for (pxint_t r = 0; r < height; r++){
        for (pxint_t c = 0; c < width; c++){
            ObjectID cell = (*this)[r + box.min_y][c + box.min_x];
            matrix[r][c] = cell == id ? 1 : 0;
        }
    }
    return matrix;
}



std::string CellMatrix::dump() const{
    pxint_t width = m_width;
    pxint_t height = m_height;

    std::string str;
    str += "{\n";
    for (pxint_t r = 0; r < height; r++){
        str += "   ";
        for (pxint_t c = 0; c < width; c++){
            str += " ";
            str += std::to_string((int)(*this)[r][c]);
        }
        str += "\n";
    }
    str += "}\n";
    return str;
}




}
