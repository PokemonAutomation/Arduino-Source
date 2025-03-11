/*  Image Resolution
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <ostream>
#include "ImageResolution.h"

namespace PokemonAutomation{


std::string Resolution::to_string() const{
    return std::to_string(width) + " x " + std::to_string(height);
}

std::ostream& operator<<(std::ostream& os, const Resolution& resolution){
    os << resolution.width << " x " << resolution.height;
    return os;
}

std::string aspect_ratio_as_string(const Resolution& resolution){
    size_t w = resolution.width;
    size_t h = resolution.height;
    if (w <= 0 || h <= 0){
        return "";
    }
    size_t gcd;
    while (true){
        if (h == 0){
            gcd = w;
            break;
        }
        w %= h;
        if (w == 0){
            gcd = h;
            break;
        }
        h %= w;
    }
    w = resolution.width;
    h = resolution.height;
    w /= gcd;
    h /= gcd;
    return "(" + std::to_string(w) + ":" + std::to_string(h) + ")";
}




}
