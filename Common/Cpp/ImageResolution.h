/*  Image Resolution
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ImageResolution_H
#define PokemonAutomation_ImageResolution_H

#include <string>

namespace PokemonAutomation{


struct Resolution{
    size_t width = 0;
    size_t height = 0;

    Resolution() = default;
    Resolution(size_t p_width, size_t p_height)
        : width(p_width)
        , height(p_height)
    {}

    explicit operator bool() const{ return width != 0 && height != 0; }

    bool operator==(const Resolution& x) const{
        return width == x.width && height == x.height;
    }
    bool operator!=(const Resolution& x) const{
        return width != x.width || height != x.height;
    }

    friend std::ostream& operator<<(std::ostream& os, const Resolution& resolution);
};

std::string aspect_ratio_as_string(const Resolution& resolution);



}
#endif
