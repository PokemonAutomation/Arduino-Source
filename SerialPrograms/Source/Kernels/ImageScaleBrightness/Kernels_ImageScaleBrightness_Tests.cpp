/*  Image Scale Brightness Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include <format>
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Tests/GuardPageImage.h"
#include "Tests/ImageUnitTest.h"
#include "Kernels_ImageScaleBrightness_Tests.h"

namespace PokemonAutomation{
namespace Kernels{


//  The scale_brightness() implementations, defined in Kernels_ImageScaleBrightness_*.cpp.
//  `scale_brightness()` in Kernels_ImageScaleBrightness.cpp declares the same functions
//  and calls the best one for the CPU. These tests call them directly, so that every
//  implementation this CPU supports is tested.
void scale_brightness_Default    (size_t width, size_t height, uint32_t* image, size_t bytes_per_row, float scaleR, float scaleG, float scaleB);
void scale_brightness_x64_SSE41  (size_t width, size_t height, uint32_t* image, size_t bytes_per_row, float scaleR, float scaleG, float scaleB);
void scale_brightness_x64_AVX2   (size_t width, size_t height, uint32_t* image, size_t bytes_per_row, float scaleR, float scaleG, float scaleB);
void scale_brightness_x64_AVX512 (size_t width, size_t height, uint32_t* image, size_t bytes_per_row, float scaleR, float scaleG, float scaleB);
void scale_brightness_arm64_NEON (size_t width, size_t height, uint32_t* image, size_t bytes_per_row, float scaleR, float scaleG, float scaleB);


namespace{

//  One scale_brightness() implementation to test: its name in the test name, and the function.
struct ScaleBrightnessImplementation{
    using Function = void (*)(
        size_t width, size_t height, uint32_t* image, size_t bytes_per_row,
        float scaleR, float scaleG, float scaleB
    );

    std::string name;
    Function scale_brightness;
};

//  The (R, G, B) scales to test: below 1, above 1 so that some channels reach the 255
//  cap, exactly 1, and a negative scale, which turns the channel to 0. Scales 0.5 and 2.5
//  give exact .5 ties on odd channel values.
constexpr float SCALES[][3] = {
    {1.2f, 1.3f, 0.5f},
    {0.7f, 2.5f, 1.0f},
    {1.0f, -1.0f, 3.0f},
};

//  Whether `result` is right for one channel `value` scaled by `scale`. The scaled value
//  is rounded to the nearest integer and capped at 255. A negative scale counts as 0. For
//  example, with scale 1.3 a channel of 102 becomes 133 (132.6 rounded), and a channel of
//  200 becomes 255. At an exact tie like 50.5, both 50 and 51 are right: Default rounds
//  ties up, but the SIMD code rounds them to even. That is a 1-unit error we accept.
bool is_right_channel(uint32_t result, uint32_t value, float scale){
    const float scaled = (float)value * std::max(scale, 0.0f);
    const float down = std::floor(scaled);
    auto capped = [](float x){
        return std::min((uint32_t)x, (uint32_t)255);
    };
    if (scaled - down == 0.5f){
        return result == capped(down) || result == capped(down + 1);
    }
    return result == capped(std::round(scaled));
}

//  Whether `result` is right for `pixel`: alpha does not change, and each of R, G and B
//  passes `is_right_channel()` with its scale.
bool is_right_pixel(uint32_t result, uint32_t pixel, float scaleR, float scaleG, float scaleB){
    return (result & 0xff000000) == (pixel & 0xff000000)
        && is_right_channel((result >> 16) & 0xff, (pixel >> 16) & 0xff, scaleR)
        && is_right_channel((result >> 8) & 0xff, (pixel >> 8) & 0xff, scaleG)
        && is_right_channel(result & 0xff, pixel & 0xff, scaleB);
}


//  Check one scale_brightness() implementation on a real image, for each of `SCALES`:
//  scale a copy of the image in place and check every pixel with `is_right_pixel()`.
//  The copy is a guard-page image, so the test also crashes if the implementation
//  reads or writes past the last pixel.
class Test_ImageScaleBrightness : public ImageUnitTest{
public:
    Test_ImageScaleBrightness(
        const ScaleBrightnessImplementation& implementation,
        const std::string& image
    )
        : ImageUnitTest("Kernels::ImageScaleBrightness - " + implementation.name + " - " + image, {image})
        , m_implementation(implementation)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        const ImageRGB32& image = images[0];

        for (const auto& scale : SCALES){
            GuardPageImageRGB32 scaled(image);
            m_implementation.scale_brightness(
                scaled.width(), scaled.height(), scaled.data(), scaled.bytes_per_row(),
                scale[0], scale[1], scale[2]
            );
            for (size_t y = 0; y < image.height(); y++){
                for (size_t x = 0; x < image.width(); x++){
                    const uint32_t pixel = image.pixel(x, y);
                    if (!is_right_pixel(scaled.pixel(x, y), pixel, scale[0], scale[1], scale[2])){
                        return std::format(
                            "Error: scales ({}, {}, {}), pixel ({}, {}) is {:08x}: got {:08x}, which is not rounded to nearest.",
                            scale[0], scale[1], scale[2], x, y, pixel, scaled.pixel(x, y)
                        );
                    }
                }
            }
        }
        return true;
    }

private:
    ScaleBrightnessImplementation m_implementation;
};

}



//  Add a test for each image and each scale_brightness() implementation that this build
//  has and this CPU supports, including Default.
void add_tests_ImageScaleBrightness(UnitTestDatabase& database){
    std::vector<ScaleBrightnessImplementation> implementations{
        {"Default", scale_brightness_Default},
    };
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        implementations.push_back({"x64_SSE41", scale_brightness_x64_SSE41});
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        implementations.push_back({"x64_AVX2", scale_brightness_x64_AVX2});
    }
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        implementations.push_back({"x64_AVX512", scale_brightness_x64_AVX512});
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        implementations.push_back({"arm64_NEON", scale_brightness_arm64_NEON});
    }
#endif

    //  Images in the CommandLineTests repo. Their widths cover every remainder mod 4, so
    //  the leftover pixels after the 2- and 4-pixel loops are tested too.
    const std::vector<std::string> images{
        "NintendoSwitch/Switch2/ghostassassin/system_settings_display_dark.png",    //  1916 x 1078
        "OCR/Lv.1-0.png",                                                           //  237 x 77
        "PokemonSwSh/SelectionArrowFinder/mac_neon_error_1.png",                    //  106 x 410
        "OCR/german-nature-sanft.png",                                              //  511 x 91
    };

    for (const ScaleBrightnessImplementation& implementation : implementations){
        for (const std::string& image : images){
            database.add<Test_ImageScaleBrightness>(implementation, image);
        }
    }
}



}
}
