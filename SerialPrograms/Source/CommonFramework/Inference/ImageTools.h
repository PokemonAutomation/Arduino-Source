/*  Image Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ImageTools_H
#define PokemonAutomation_ImageTools_H

#include <ostream>
#include <QImage>

namespace PokemonAutomation{

struct FloatPixel{
    double r;
    double g;
    double b;

    FloatPixel() : r(0), g(0), b() {}
    FloatPixel(double red, double green, double blue)
        : r(red), g(green), b(blue)
    {}
    FloatPixel(QRgb pixel)
        : r(qRed(pixel)), g(qGreen(pixel)), b(qBlue(pixel))
    {}

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



struct InferenceBox;

QImage extract_box(const QImage& image, const InferenceBox& box);
FloatPixel pixel_average(const QImage& image);
FloatPixel pixel_average_normalized(const QImage& image);
FloatPixel pixel_stddev(const QImage& image);


}
#endif

