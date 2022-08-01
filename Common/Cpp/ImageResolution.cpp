/*  Image Resolution
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ImageResolution.h"

namespace PokemonAutomation{


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
