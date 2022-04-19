/*  Image
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_Image_H
#define PokemonAutomation_CommonFramework_Image_H

#include <stddef.h>
#include <stdint.h>
#include "Common/Compiler.h"

class QString;
class QImage;

namespace PokemonAutomation{



class ImageRef{
public:
    ImageRef();

    ImageRef(uint32_t* ptr, size_t bytes_per_row, size_t width, size_t height);
    ImageRef(QImage& image);

    operator bool() const{ return m_data != nullptr; }

    uint32_t* data() const{ return m_data; }
    size_t bytes_per_row() const{ return m_bytes_per_row; }
    size_t width() const{ return m_width; }
    size_t height() const{ return m_height; }

    uint32_t& pixel(size_t x, size_t y) const{
        return *(uint32_t*)((const char*)m_data + x * sizeof(uint32_t) + y * m_bytes_per_row);
    }


public:
    ImageRef sub_image(size_t min_x, size_t min_y, size_t width, size_t height) const;


public:
    void save(const QString& path) const;
    QImage to_qimage() const;
    QImage scaled_to_qimage(size_t width, size_t height) const;


private:
    uint32_t* m_data;
    size_t m_bytes_per_row;
    size_t m_width;
    size_t m_height;
};



class ConstImageRef{
public:
    ConstImageRef();
    ConstImageRef(const ImageRef& image)
        : m_data(image.data())
        , m_bytes_per_row(image.bytes_per_row())
        , m_width(image.width())
        , m_height(image.height())
    {}

    ConstImageRef(const uint32_t* ptr, size_t bytes_per_row, size_t width, size_t height);
    ConstImageRef(const QImage& image);

    operator bool() const{ return m_data != nullptr; }

    const uint32_t* data() const{ return m_data; }
    size_t bytes_per_row() const{ return m_bytes_per_row; }
    size_t width() const{ return m_width; }
    size_t height() const{ return m_height; }

    uint32_t pixel(size_t x, size_t y) const{
        return *(const uint32_t*)((const char*)m_data + x * sizeof(uint32_t) + y * m_bytes_per_row);
    }


public:
    void save(const QString& path) const;
    QImage to_qimage() const;
    QImage scaled_to_qimage(size_t width, size_t height) const;


public:
    ConstImageRef sub_image(size_t min_x, size_t min_y, size_t width, size_t height) const;


private:
    const uint32_t* m_data;
    size_t m_bytes_per_row;
    size_t m_width;
    size_t m_height;
};





}
#endif
