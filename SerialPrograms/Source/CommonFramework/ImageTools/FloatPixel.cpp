/*  Float Pixel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "FloatPixel.h"

namespace PokemonAutomation{


std::string FloatPixel::to_string() const{
    return "{" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + "}";
}
double FloatPixel::stddev() const{
    double mean = (r + g + b) / 3;
    double rd = (r - mean);
    double gd = (g - mean);
    double bd = (b - mean);
    return std::sqrt((rd*rd + gd*gd + bd*bd) / 2);
}

FloatPixel abs(const FloatPixel& x){
    return FloatPixel{
        x.r < 0 ? -x.r : x.r,
        x.g < 0 ? -x.g : x.g,
        x.b < 0 ? -x.b : x.b,
    };
}
double euclidean_distance(const FloatPixel& x, const FloatPixel& y){
    FloatPixel p = x - y;
    p *= p;
    return std::sqrt(p.r + p.g + p.b);
}


}
