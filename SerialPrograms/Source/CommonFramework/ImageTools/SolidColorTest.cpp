/*  Solid Color Test
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "SolidColorTest.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


bool is_white(
    const ImageStats& stats,
    double min_rgb_sum,
    double max_stddev_sum
){
    if (stats.average.sum() < min_rgb_sum){
        return false;
    }
    return is_solid(stats, {0.333333, 0.333333, 0.333333}, 0.1, max_stddev_sum);
}
bool is_black(
    const ImageStats& stats,
    double max_rgb_sum,
    double max_stddev_sum
){
    double average = stats.average.sum();
    double stddev = stats.stddev.sum();
    return average <= max_rgb_sum && stddev <= max_stddev_sum;
}
bool is_grey(
    const ImageStats& stats,
    double min_rgb_sum, double max_rgb_sum,
    double max_stddev_sum
){
    double sum = stats.average.sum();
    if (sum < min_rgb_sum || sum > max_rgb_sum){
        return false;
    }
    return is_solid(stats, {0.333333, 0.333333, 0.333333}, 0.1, max_stddev_sum);
}
bool is_solid(
    const ImageStats& stats,
    const FloatPixel& expected_color_ratio,
    double max_euclidean_distance,
    double max_stddev_sum
){
    double average = stats.average.sum();
    double stddev = stats.stddev.sum();
    if (stddev > max_stddev_sum){
//        cout << "bad stddev = " << stddev << endl;
        return false;
    }

    FloatPixel actual = stats.average / average;
    double distance = euclidean_distance(actual, expected_color_ratio);
//    cout << "distance = " << distance << endl;

    return distance <= max_euclidean_distance;
}


}
