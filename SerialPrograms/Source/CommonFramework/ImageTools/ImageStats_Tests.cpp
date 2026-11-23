/*  Image Stats Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "ImageStats.h"
#include "ImageStats_Tests.h"

namespace PokemonAutomation{

namespace{



//  Return an empty string if `stats` has the expected values, or else a message
//  that names the first value that differs.
std::string check_stats(
    const ImageStats& stats,
    uint64_t expected_count, FloatPixel expected_average, FloatPixel expected_stddev
){
    auto pixels_close = [](const FloatPixel& x, const FloatPixel& y){
        return std::abs(x.r - y.r) < 0.001
            && std::abs(x.g - y.g) < 0.001
            && std::abs(x.b - y.b) < 0.001;
    };
    if (stats.count != expected_count){
        return "count: expected " + std::to_string(expected_count) + ", received " + std::to_string(stats.count);
    }
    if (!pixels_close(stats.average, expected_average)){
        return "average: expected " + expected_average.to_string() + ", received " + stats.average.to_string();
    }
    if (!pixels_close(stats.stddev, expected_stddev)){
        return "stddev: expected " + expected_stddev.to_string() + ", received " + stats.stddev.to_string();
    }
    return "";
}



//  Check that `image_border_stats()` reads each border pixel once, corners included,
//  and no inner pixel.
class Test_ImageBorderStats : public UnitTest{
public:
    Test_ImageBorderStats()
        : UnitTest("CommonFramework::ImageStats - image_border_stats()")
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        const uint32_t gray = (uint32_t)Color(100, 100, 100);
        const uint32_t red = (uint32_t)Color(200, 0, 50);
        const uint32_t white = (uint32_t)Color(255, 255, 255);

        //  A 4x3 image with a gray border, except for a red right column, and
        //  2 white inner pixels:
        //      G G G R
        //      G W W R
        //      G G G R
        //  The 10 border pixels are 7 gray and 3 red. So the average is
        //  (7 * gray + 3 * red) / 10 = (130, 70, 85). Each channel's sample
        //  variance is (7 * d_gray^2 + 3 * d_red^2) / 9, where d is the distance
        //  to the average: 21000/9 for red and green, 5250/9 for blue.
        {
            ImageRGB32 image(4, 3);
            image.fill(gray);
            for (size_t r = 0; r < 3; r++){
                image.pixel(3, r) = red;
            }
            image.pixel(1, 1) = white;
            image.pixel(2, 1) = white;

            std::string error = check_stats(
                image_border_stats(image), 10,
                FloatPixel(130, 70, 85),
                FloatPixel(std::sqrt(21000.0 / 9), std::sqrt(21000.0 / 9), std::sqrt(5250.0 / 9))
            );
            if (!error.empty()){
                return "4x3 image, " + error;
            }
        }

        //  In a one-pixel-tall or one-pixel-wide image, every pixel is on the
        //  border, and each one counts once: values 10, 20, 30 give average 20
        //  and sample stddev 10.
        const uint32_t line[3] = {
            (uint32_t)Color(10, 10, 10),
            (uint32_t)Color(20, 20, 20),
            (uint32_t)Color(30, 30, 30),
        };
        {
            ImageRGB32 image(3, 1);
            for (size_t c = 0; c < 3; c++){
                image.pixel(c, 0) = line[c];
            }
            std::string error = check_stats(
                image_border_stats(image), 3, FloatPixel(20, 20, 20), FloatPixel(10, 10, 10)
            );
            if (!error.empty()){
                return "3x1 image, " + error;
            }
        }
        {
            ImageRGB32 image(1, 3);
            for (size_t r = 0; r < 3; r++){
                image.pixel(0, r) = line[r];
            }
            std::string error = check_stats(
                image_border_stats(image), 3, FloatPixel(20, 20, 20), FloatPixel(10, 10, 10)
            );
            if (!error.empty()){
                return "1x3 image, " + error;
            }
        }

        //  A single pixel has no stddev, so the function returns empty stats.
        {
            ImageRGB32 image(1, 1);
            image.fill(gray);
            if (image_border_stats(image).count != 0){
                return "1x1 image: expected count 0";
            }
        }

        return true;
    }
};



}



void add_tests_ImageStats(UnitTestDatabase& database){
    database.add<Test_ImageBorderStats>();
}



}
