/*  Flag Matrix
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "FillMatrix.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


FillMatrix::FillMatrix(FillMatrix&& x)
    : m_width(x.m_width)
    , m_height(x.m_height)
    , m_cells(std::move(x.m_cells))
{
    x.m_width = 0;
    x.m_height = 0;
}
FillMatrix& FillMatrix::operator=(FillMatrix&& x){
    m_width = x.m_width;
    m_height = x.m_height;
    m_cells = std::move(x.m_cells);
    x.m_width = 0;
    x.m_height = 0;
    return *this;
}

FillMatrix::FillMatrix()
    : m_width(0)
    , m_height(0)
{}
FillMatrix::FillMatrix(size_t width, size_t height)
    : m_width(width)
    , m_height(height)
    , m_cells(width * height)
{}
FillMatrix::FillMatrix(const QImage& image)
    : FillMatrix(
        image.isNull() ? 0 : image.width(),
        image.isNull() ? 0 : image.height()
    )
{}

FillMatrix FillMatrix::extract(PixelBox box, ObjectID id) const{
    int width = box.width();
    int height = box.height();
    FillMatrix matrix(width, height);
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
            ObjectID cell = (*this)[r + box.min_y][c + box.min_x];
            matrix[r][c] = cell == id ? 1 : 0;
        }
    }
    return matrix;
}



std::string FillMatrix::dump() const{
    int width = m_width;
    int height = m_height;

    std::string str;
    str += "{\n";
    for (int r = 0; r < height; r++){
        str += "   ";
        for (int c = 0; c < width; c++){
            str += " ";
            str += std::to_string((int)(*this)[r][c]);
        }
        str += "\n";
    }
    str += "}\n";
    return str;
}




}
