/*  Float Pixel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_FloatPixel_H
#define PokemonAutomation_CommonFramework_FloatPixel_H

#include <ostream>
#include <QImage>

namespace PokemonAutomation{


struct FloatPixel{
    double r;
    double g;
    double b;

    FloatPixel() : r(0), g(0), b(0) {}
    FloatPixel(double red, double green, double blue)
        : r(red), g(green), b(blue)
    {}
    FloatPixel(QRgb pixel)
        : r(qRed(pixel)), g(qGreen(pixel)), b(qBlue(pixel))
    {}

    QRgb round() const{
        return qRgb((uint8_t)(r + 0.5), (uint8_t)(g + 0.5), (uint8_t)(b + 0.5));
    }

    double sum() const{ return r + g + b; }
    QString to_string() const;
    double stddev() const;
};

inline std::ostream& operator<<(std::ostream& stream, const FloatPixel& pixel){
    stream << "{" << pixel.r << ", " << pixel.g << ", " << pixel.b << "}";
    return stream;
}


inline void operator+=(FloatPixel& x, const FloatPixel& y){
    x.r += y.r;
    x.g += y.g;
    x.b += y.b;
}
inline void operator-=(FloatPixel& x, const FloatPixel& y){
    x.r -= y.r;
    x.g -= y.g;
    x.b -= y.b;
}
inline void operator*=(FloatPixel& x, const FloatPixel& y){
    x.r *= y.r;
    x.g *= y.g;
    x.b *= y.b;
}
inline void operator/=(FloatPixel& x, double y){
    x.r /= y;
    x.g /= y;
    x.b /= y;
}
inline FloatPixel operator+(const FloatPixel& x, const FloatPixel& y){
    return FloatPixel{
        x.r + y.r,
        x.g + y.g,
        x.b + y.b,
    };
}
inline FloatPixel operator-(const FloatPixel& x, const FloatPixel& y){
    return FloatPixel{
        x.r - y.r,
        x.g - y.g,
        x.b - y.b,
    };
}
inline FloatPixel operator*(const FloatPixel& x, const FloatPixel& y){
    return FloatPixel{
        x.r * y.r,
        x.g * y.g,
        x.b * y.b,
    };
}
inline FloatPixel operator/(const FloatPixel& x, double y){
    return FloatPixel{
        x.r / y,
        x.g / y,
        x.b / y,
    };
}

FloatPixel abs(const FloatPixel& x);
double euclidean_distance(const FloatPixel& x, const FloatPixel& y);


}

#endif
