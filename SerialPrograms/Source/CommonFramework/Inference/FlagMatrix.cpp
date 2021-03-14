/*  Flag Matrix
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "FlagMatrix.h"

namespace PokemonAutomation{


FlagMatrix::FlagMatrix(FlagMatrix&& x)
    : m_width(x.m_width)
    , m_height(x.m_height)
    , m_cells(std::move(x.m_cells))
{
    x.m_width = 0;
    x.m_height = 0;
}
FlagMatrix& FlagMatrix::operator=(FlagMatrix&& x){
    m_width = x.m_width;
    m_height = x.m_height;
    m_cells = std::move(x.m_cells);
    x.m_width = 0;
    x.m_height = 0;
    return *this;
}

FlagMatrix::FlagMatrix(size_t width, size_t height)
    : m_width(width)
    , m_height(height)
    , m_cells(width * height)
{}
FlagMatrix::FlagMatrix(const QImage& image)
    : FlagMatrix(
        image.isNull() ? 0 : image.width(),
        image.isNull() ? 0 : image.height()
    )
{}



}
