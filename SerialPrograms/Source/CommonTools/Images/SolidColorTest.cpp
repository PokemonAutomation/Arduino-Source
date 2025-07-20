/*  Solid Color Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "SolidColorTest.h"

#include <sstream>
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
    return is_solid(stats, {0.333333, 0.333333, 0.333333}, 0.08, max_stddev_sum);
}
bool is_black(
    const ImageStats& stats,
    double max_rgb_sum,
    double max_stddev_sum
){
    double average = stats.average.sum();
    double stddev = stats.stddev.sum();
    if (PreloadSettings::debug().COLOR_CHECK){
        cout << "is_black(): stats.average " << stats.average.to_string() << "(sum " << average << ") stats.stddev " << stats.stddev.to_string()
             << "(sum " << stddev << ") max_rgb_sum " << max_rgb_sum << " max_stddev_sum " << max_stddev_sum << endl;
    }
//    cout << stats.average << stats.stddev << endl;
    return average <= max_rgb_sum && stddev <= max_stddev_sum;
}
bool is_grey(
    const ImageStats& stats,
    double min_rgb_sum, double max_rgb_sum,
    double max_stddev_sum
){
    double average = stats.average.sum();
    if (average < min_rgb_sum || average > max_rgb_sum){
        return false;
    }

    double stddev = stats.stddev.sum();
    if (stddev > max_stddev_sum){
//       cout << "bad stddev = " << stddev << endl;
        return false;
    }

    //  If it's too dark, color ratios will be unstable.
    if (average < 100){
        return true;
    }

    FloatPixel actual = stats.average / average;
    double distance = euclidean_distance(actual, {0.333333, 0.333333, 0.333333});
    return distance <= 0.1;
}
bool is_solid(
    const ImageStats& stats,
    const FloatPixel& expected_color_ratio,
    double max_euclidean_distance,
    double max_stddev_sum
){
    double stddev = stats.stddev.sum();
    if (stddev > max_stddev_sum){
//       cout << "bad stddev = " << stddev << endl;
        return false;
    }

    double average = stats.average.sum();
    FloatPixel actual = stats.average / average;
    double distance = euclidean_distance(actual, expected_color_ratio);
//   cout << "actual color ratio " << actual << endl;
//   cout << "distance = " << distance << endl;

    return distance <= max_euclidean_distance;
}

bool ImageSolidCheck::check(const ImageViewRGB32& frame) const{
    ImageStats stats = image_stats(extract_box_reference(frame, box));
    return is_solid(stats, expected_color_ratio, max_euclidean_distance, max_stddev_sum);
}

std::string ImageSolidCheck::debug_string(const ImageViewRGB32& frame) const{
    std::ostringstream oss;
    ImageStats stats = image_stats(extract_box_reference(frame, box));
    oss << "Box (" << box.x << ", " << box.y << ", " << box.width << ", " << box.height << ") ";
    
    double average = stats.average.sum();
    FloatPixel actual_ratio = stats.average / average;
    oss << "expected color ratio: " << expected_color_ratio.to_string() << " ";
    oss << "actual color ratio: " << actual_ratio.to_string() << " ";
    oss << "dist: " << euclidean_distance(actual_ratio, expected_color_ratio) << " ";

    double actual_stddev = stats.stddev.sum();
    oss << "max stddev sum: " << max_stddev_sum << " actual stddev sum: " << actual_stddev;

    return oss.str();
}


}
