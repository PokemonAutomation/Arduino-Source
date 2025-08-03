/*  Solid Color Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_SolidColorTest_H
#define PokemonAutomation_CommonTools_SolidColorTest_H

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


bool is_white(
    const ImageStats& stats,
    double min_rgb_sum = 500,
    double max_stddev_sum = 10
);
bool is_black(
    const ImageStats& stats,
    double max_rgb_sum = 100,
    double max_stddev_sum = 10
);
bool is_grey(
    const ImageStats& stats,
    double min_rgb_sum, double max_rgb_sum,
    double max_stddev_sum = 10
);

// expected_color_ratio: ratio of color channels to match, 
// for a color {r, g, b}: the color ratio is {r/(r+g+b), g/(r+g+b), b/(r+g+b)}
// the sum of the ratios should add up to 1.0.
// e.g. if a color is (127, 127, 254), it's color ratio is (0.25, 0.25, 0.5)
bool is_solid(
    const ImageStats& stats,
    const FloatPixel& expected_color_ratio,
    double max_euclidean_distance = 0.15,
    double max_stddev_sum = 10
);


inline bool is_white(
    const ImageViewRGB32& image,
    double min_rgb_sum = 500,
    double max_stddev_sum = 10
){
    return is_white(image_stats(image), min_rgb_sum, max_stddev_sum);
}
inline bool is_black(
    const ImageViewRGB32& image,
    double max_rgb_sum = 100,
    double max_stddev_sum = 10
){
    ImageStats stats = image_stats(image);
//    cout << stats.average << stats.stddev << endl;
//    cout << is_black(stats, max_rgb_sum, max_stddev_sum) << endl;
    return is_black(stats, max_rgb_sum, max_stddev_sum);
}
inline bool is_grey(
    const ImageViewRGB32& image,
    double min_rgb_sum, double max_rgb_sum,
    double max_stddev_sum = 10
){
    return is_grey(image_stats(image), min_rgb_sum, max_rgb_sum, max_stddev_sum);
}

// expected_color_ratio: ratio of color channels to match, e.g. if a color is (127, 127, 254), it's color ratio is (0.25, 0.25, 0.5)
inline bool is_solid(
    const ImageViewRGB32& image,
    const FloatPixel& expected_color_ratio,
    double max_euclidean_distance = 0.15,
    double max_stddev_sum = 10
){
    return is_solid(image_stats(image), expected_color_ratio, max_euclidean_distance, max_stddev_sum);
}

// A convenience struct to do solid checks on images.
struct ImageSolidCheck{
    ImageFloatBox box;
    FloatPixel expected_color_ratio;
    double max_euclidean_distance = 0.15;
    double max_stddev_sum = 10;

    ImageSolidCheck(
        double x, double y, double width, double height,
        double r, double g, double b,
        double max_distance = 0.15, double max_stddev_sum = 10)
        : box(x, y, width, height)
        , expected_color_ratio(r, g, b)
        , max_euclidean_distance(max_distance)
        , max_stddev_sum(max_stddev_sum) {}
    
    // Check if the area on the image is a solid color.
    bool check(const ImageViewRGB32& image) const;

    // Return a debug string on the checks performed on the image.
    std::string debug_string(const ImageViewRGB32& image) const;
};




}
#endif
