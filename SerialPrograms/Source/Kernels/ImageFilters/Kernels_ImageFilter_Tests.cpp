/*  Image Filters Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#ifdef PA_AutoDispatch_arm64_20_M1
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64x8_arm64_NEON.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Core_64x8_arm64_NEON.h"
#endif
//#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64xH_Default.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range.h"
#include "Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean.h"
#include "Kernels_ImageFilter_Tests.h"
#include "Tests/TestUtils.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::flush;
namespace PokemonAutomation{
namespace Kernels{

class Test_ImageFilterRGB32Range : public UnitTest{
public:
    Test_ImageFilterRGB32Range(
        const std::string& image
    )
        : UnitTest("Kernels::ImageFilterRGB32Range - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);

        const size_t width = image.width(), height = image.height();
        cout << "Testing filter_rgb32_range(), image size " << width << " x " << height << endl;

        Color min_color(0, 0, 0);
        Color max_color(63, 63, 63);
        // Color max_color(238, 24, 42);

        const uint32_t mins = uint32_t(min_color);
        const uint32_t maxs = uint32_t(max_color);

        ImageRGB32 image_out(image.width(), image.height());
        ImageRGB32 image_out_2(image.width(), image.height());
        size_t pixels_in_range = 0;

        const bool replace_color_within_range = true;
        auto time_start = current_time();
        // auto new_image = filter_rgb32_range(image, mins, maxs, COLOR_WHITE, replace_color_within_range);
        pixels_in_range = Kernels::filter_rgb32_range(
            image.data(), image.bytes_per_row(), image.width(), image.height(),
            image_out.data(), image_out.bytes_per_row(),
            (uint32_t)COLOR_WHITE, replace_color_within_range,
            mins, maxs
        );
        auto time_end = current_time();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
        auto ms = ns / 1000000.;
        cout << "One filter time: " << ms << " ms" << endl;

        size_t pixels_in_range_2 = Kernels::filter_rgb32_range(
            image.data(), image.bytes_per_row(), image.width(), image.height(),
            image_out_2.data(), image_out_2.bytes_per_row(),
            (uint32_t)COLOR_WHITE, !replace_color_within_range,
            mins, maxs
        );

        TEST_RESULT_EQUAL_STR(pixels_in_range, pixels_in_range_2);

        size_t actual_num_pixels_in_range = 0;
        size_t error_count = 0;
        for (size_t y = 0; y < height; y++){
            for (size_t x = 0; x < width; x++){
                const Color color(image.pixel(x, y));
                const Color new_color(image_out.pixel(x, y));
                const Color new_color_2(image_out_2.pixel(x, y));
                bool in_range = (min_color.alpha() <= color.alpha() && color.alpha() <= max_color.alpha());
                in_range = in_range && (min_color.red() <= color.red() && color.red() <= max_color.red());
                in_range = in_range && (min_color.green() <= color.green() && color.green() <= max_color.green());
                in_range = in_range && (min_color.blue() <= color.blue() && color.blue() <= max_color.blue());
                actual_num_pixels_in_range += in_range;
                if (error_count < 10){
                    // Print first 10 errors:
                    if (in_range && new_color != COLOR_WHITE){
                        cout << "Error: wrong filter result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should be in range but not found by the function" << endl;
                        ++error_count;
                    } else if (in_range == false && new_color != color){
                        cout << "Error: wrong filter result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should not be changed by the function" << endl;
                        ++error_count;
                    }

                    if (in_range && new_color_2 != color){
                        cout << "Error: wrong inverse filter result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should not be changed by the function" << endl;
                        ++error_count;
                    } else if (in_range == false && new_color_2 != COLOR_WHITE){
                        cout << "Error: wrong inverse filter result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should not be in range but not found by the function" << endl;
                        ++error_count;
                    }
                }
            }
        }
        cout << "Found " << actual_num_pixels_in_range << " pixels in range" << endl;
        if (pixels_in_range != actual_num_pixels_in_range){
            cout << "Error: wrong pixels in range: " << pixels_in_range << " actual: " << actual_num_pixels_in_range << endl;
            return false;
        }

        if (error_count){
            return false;
        }

        // We try to wait for three seconds:
        const size_t num_iters = size_t(3000 / ms);
        time_start = current_time();
        for (size_t i = 0; i < num_iters; i++){
            Kernels::filter_rgb32_range(
                image.data(), image.bytes_per_row(), image.width(), image.height(),
                image_out.data(), image_out.bytes_per_row(),
                (uint32_t)COLOR_WHITE, replace_color_within_range,
                mins, maxs
            );
        }
        time_end = current_time();
        ms = (double)std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
        cout << "Running " << num_iters << " iters, avg filter time: " << ms / num_iters << " ms" << endl;

        return true;
    };

private:
    std::string m_image;
};


class Test_ImageFilterRGB32Euclidean : public UnitTest{
public:
    Test_ImageFilterRGB32Euclidean(
        const std::string& image
    )
        : UnitTest("Kernels::ImageFilterRGB32Euclidean - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);

        const size_t width = image.width();
        const size_t height = image.height();
        cout << "Testing test_kernels_FilterRGB32Euclidean(), image size " << width << " x " << height << endl;

        Color middle_color = Color(image.pixel(width / 2, height / 2));
        cout << "Expected color: " << middle_color.to_string() << endl;

        double max_dist = 50.0;
        size_t max_dist2 = size_t(max_dist * max_dist);

        ImageRGB32 image_out(image.width(), image.height());
        ImageRGB32 image_out_2(image.width(), image.height());
        size_t pixels_in_range = 0;

        const bool replace_color_within_range = true;
        auto time_start = current_time();
        pixels_in_range = Kernels::filter_rgb32_euclidean(
            image.data(), image.bytes_per_row(), image.width(), image.height(),
            image_out.data(), image_out.bytes_per_row(),
            (uint32_t)COLOR_WHITE, replace_color_within_range,
            uint32_t(middle_color), max_dist
        );
        auto time_end = current_time();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
        auto ms = ns / 1000000.;
        cout << "One filter time: " << ms << " ms" << endl;

        size_t pixels_in_range_2 = Kernels::filter_rgb32_euclidean(
            image.data(), image.bytes_per_row(), image.width(), image.height(),
            image_out_2.data(), image_out_2.bytes_per_row(),
            (uint32_t)COLOR_WHITE, !replace_color_within_range,
            uint32_t(middle_color), max_dist
        );

        TEST_RESULT_EQUAL_STR(pixels_in_range, pixels_in_range_2);

        size_t actual_num_pixels_in_range = 0;
        size_t error_count = 0;
        for (size_t y = 0; y < height; y++){
            for (size_t x = 0; x < width; x++){
                const Color color(image.pixel(x, y));
                const Color new_color(image_out.pixel(x, y));
                const Color new_color_2(image_out_2.pixel(x, y));
                int red_dif = (color.red() - middle_color.red());
                int green_dif = (color.green() - middle_color.green());
                int blue_dif = (color.blue() - middle_color.blue());
                size_t color_dist2 = red_dif * red_dif + green_dif * green_dif + blue_dif * blue_dif;
                bool in_range = color_dist2 <= max_dist2;
                actual_num_pixels_in_range += in_range;
                if (error_count < 10){
                    // Print first 10 errors:
                    if (in_range && new_color != COLOR_WHITE){
                        cout << "Error: wrong filter result: old color " << color.to_string()
                            << ", (x,y) = (" << x << ", " << y << ")"
                            << ", dist2 " << color_dist2 << ", max dist2 " << max_dist2
                            << ", should be in range but not found by the function" << endl;
                        ++error_count;
                    } else if (in_range == false && new_color != color){
                        cout << "Error: wrong filter result: old color " << color.to_string()
                            << ", (x,y) = (" << x << ", " << y << ")"
                            << ", dist2 " << color_dist2 << ", max dist2 " << max_dist2
                            << ", should not be changed by the function" << endl;
                        ++error_count;
                    }

                    if (in_range && new_color_2 != color){
                        cout << "Error: wrong inverse filter result: old color " << color.to_string()
                            << ", (x,y) = (" << x << ", " << y << ")"
                            << ", dist2 " << color_dist2 << ", max dist2 " << max_dist2
                            << ", should not be changed by the function" << endl;
                        ++error_count;
                    } else if (in_range == false && new_color_2 != COLOR_WHITE){
                        cout << "Error: wrong inverse filter result: old color " << color.to_string()
                            << ", (x,y) = (" << x << ", " << y << ")"
                            << ", dist2 " << color_dist2 << ", max dist2 " << max_dist2
                            << ", should not be in range but not found by the function" << endl;
                        ++error_count;
                    }
                }
            }
        }
        cout << "Found " << actual_num_pixels_in_range << " pixels in range" << endl;
        if (pixels_in_range != actual_num_pixels_in_range){
            cout << "Error: wrong pixels in range: " << pixels_in_range << " actual: " << actual_num_pixels_in_range << endl;
            return false;
        }

        if (error_count){
            return false;
        }

        // We try to wait for three seconds:
        const size_t num_iters = size_t(3000 / ms);
        time_start = current_time();
        for (size_t i = 0; i < num_iters; i++){
            pixels_in_range = Kernels::filter_rgb32_euclidean(
                image.data(), image.bytes_per_row(), image.width(), image.height(),
                image_out.data(), image_out.bytes_per_row(),
                (uint32_t)COLOR_WHITE, replace_color_within_range,
                uint32_t(middle_color), max_dist
            );
        }
        time_end = current_time();
        ms = (double)std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
        cout << "Running " << num_iters << " iters, avg filter time: " << ms / num_iters << " ms" << endl;

        return true;
        
    };

private:
    std::string m_image;
};


class Test_ImageToBlackWhiteRGB32Range : public UnitTest{
public:
    Test_ImageToBlackWhiteRGB32Range(
        const std::string& image
    )
        : UnitTest("Kernels::ImageToBlackWhiteRGB32Range - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);

        const size_t width = image.width();
        const size_t height = image.height();
        cout << "Testing to_black_white_rgb32_range(), image size " << width << " x " << height << endl;

        Color min_color(0, 0, 0);
        // Color min_color(0);

        Color max_color(63, 63, 63);
        // Color max_color(255, 255, 255);
        // Color max_color(238, 24, 42);
        cout << "min color: " << min_color.to_string() << " max color: " << max_color.to_string() << endl;

        const uint32_t mins = uint32_t(min_color);
        const uint32_t maxs = uint32_t(max_color);

        ImageRGB32 image_out(image.width(), image.height());
        ImageRGB32 image_out_2(image.width(), image.height());
        size_t pixels_in_range = 0;

        const bool in_range_black = true;
        auto time_start = current_time();
        // auto new_image = filter_rgb32_range(image, mins, maxs, COLOR_WHITE, replace_color_within_range);
        pixels_in_range = Kernels::to_blackwhite_rgb32_range(
            image.data(), image.bytes_per_row(), image.width(), image.height(),
            image_out.data(), image_out.bytes_per_row(),
            in_range_black,
            mins, maxs
        );
        auto time_end = current_time();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
        auto ms = ns / 1000000.;
        cout << "One filter time: " << ms << " ms" << endl;

        size_t pixels_in_range_2 = Kernels::to_blackwhite_rgb32_range(
            image.data(), image.bytes_per_row(), image.width(), image.height(),
            image_out_2.data(), image_out_2.bytes_per_row(),
            !in_range_black,
            mins, maxs
        );

        TEST_RESULT_EQUAL_STR(pixels_in_range, pixels_in_range_2);

        size_t actual_num_pixels_in_range = 0;
        size_t error_count = 0;
        for (size_t y = 0; y < height; y++){
            for (size_t x = 0; x < width; x++){
                const Color color(image.pixel(x, y));
                const Color new_color(image_out.pixel(x, y));
                const Color new_color_2(image_out_2.pixel(x, y));
                bool in_range = (min_color.alpha() <= color.alpha() && color.alpha() <= max_color.alpha());
                in_range = in_range && (min_color.red() <= color.red() && color.red() <= max_color.red());
                in_range = in_range && (min_color.green() <= color.green() && color.green() <= max_color.green());
                in_range = in_range && (min_color.blue() <= color.blue() && color.blue() <= max_color.blue());
                actual_num_pixels_in_range += in_range;
                if (error_count < 10){
                    // Print first 10 errors:
                    if (in_range && new_color != COLOR_BLACK){
                        cout << "Error: wrong filter result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should be black due to in range but not so" << endl;
                        ++error_count;
                    } else if (in_range == false && new_color != COLOR_WHITE){
                        cout << "Error: wrong filter result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should be white due to out of range but not so" << endl;
                        ++error_count;
                    }

                    if (in_range && new_color_2 != COLOR_WHITE){
                        cout << "Error: wrong inverse filter result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should be white due to in range but not so" << endl;
                        ++error_count;
                    } else if (in_range == false && new_color_2 != COLOR_BLACK){
                        cout << "Error: wrong inverse filter result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should be black due to out of range but not so" << endl;
                        ++error_count;
                    }
                }
            }
        }
        cout << "Found " << actual_num_pixels_in_range << " pixels in range" << endl;
        if (pixels_in_range != actual_num_pixels_in_range){
            cout << "Error: wrong pixels in range: " << pixels_in_range << " actual: " << actual_num_pixels_in_range << endl;
            return false;
        }

        if (error_count){
            return false;
        }

        // We try to wait for three seconds:
        const size_t num_iters = size_t(3000 / ms);
        time_start = current_time();
        for (size_t i = 0; i < num_iters; i++){
            Kernels::to_blackwhite_rgb32_range(
                image.data(), image.bytes_per_row(), image.width(), image.height(),
                image_out.data(), image_out.bytes_per_row(),
                in_range_black,
                mins, maxs
            );
        }
        time_end = current_time();
        ms = (double)std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
        cout << "Running " << num_iters << " iters, avg filter time: " << ms / num_iters << " ms" << endl;

        return true;

    };

private:
    std::string m_image;
};



class Test_ImageFilterByMask : public UnitTest{
public:
    Test_ImageFilterByMask(
        const std::string& image
    )
        : UnitTest("Kernels::ImageFilterByMask - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);

        const size_t width = image.width(), height = image.height();
        cout << "Image width " << width << " height " << height << endl;

        const Color min_color(0, 0, 0), max_color(63, 63, 63);
        const uint32_t mins = uint32_t(min_color), maxs = uint32_t(max_color);

        auto binary_matrix = make_PackedBinaryMatrix(get_BinaryMatrixType(), width, height);
        compress_rgb32_to_binary_range(
            image.data(), image.bytes_per_row(), *binary_matrix, mins, maxs
        );
        ImageRGB32 new_image = image.copy();
        ImageRGB32 new_image_2 = image.copy();

        Color replacement_color = COLOR_WHITE;
        bool replace_zero_bits = true;

        auto time_start = current_time();
        filter_by_mask(*binary_matrix, new_image.data(), new_image.bytes_per_row(), uint32_t(replacement_color), replace_zero_bits);
        auto time_end = current_time();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
        auto ms = ns / 1000000.;
        cout << "One Filter by mask call. time: " << ms << " ms" << endl;

        filter_by_mask(*binary_matrix, new_image_2.data(), new_image_2.bytes_per_row(), uint32_t(replacement_color), !replace_zero_bits);

        size_t error_count = 0;

        for (size_t y = 0; y < height; y++){
            for (size_t x = 0; x < width; x++){
                const Color color(image.pixel(x, y));
                const Color new_color(new_image.pixel(x, y));
                const Color new_color_2(new_image_2.pixel(x, y));
                bool in_range = (min_color.alpha() <= color.alpha() && color.alpha() <= max_color.alpha());
                in_range = in_range && (min_color.red() <= color.red() && color.red() <= max_color.red());
                in_range = in_range && (min_color.green() <= color.green() && color.green() <= max_color.green());
                in_range = in_range && (min_color.blue() <= color.blue() && color.blue() <= max_color.blue());

                if (error_count <= 10){
                    if (in_range && new_color != color){
                        cout << "Error: wrong filter(replace_zero_bits) result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should not be changed due to being one bit but not so" << endl;
                        ++error_count;
                    } else if (!in_range && new_color != replacement_color){
                        cout << "Error: wrong filter(replace_zero_bits) result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should be changed due to being zero bit but not so" << endl;
                        ++error_count;
                    }

                    if (in_range && new_color_2 != replacement_color){
                        cout << "Error: wrong filter(replace_one_bits) result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should be changed due to being one bit but not so" << endl;
                        ++error_count;
                    } else if (!in_range && new_color_2 != color){
                        cout << "Error: wrong filter(replace_one_bits) result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should not be changed due to being zero bit but not so" << endl;
                        ++error_count;
                    }
                }
            }
        }

        if (error_count){
            return false;
        }

        // We try to wait for three seconds:
        const size_t num_iters = size_t(3000 / ms);
        time_start = current_time();
        for (size_t i = 0; i < num_iters; i++){
            filter_by_mask(*binary_matrix, new_image.data(), new_image.bytes_per_row(), uint32_t(replacement_color), replace_zero_bits);
        }
        time_end = current_time();
        ms = (double)std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
        cout << "Running " << num_iters << " iters, avg filter time: " << ms / num_iters << " ms" << endl;

        return true;

    };

private:
    std::string m_image;
};


class Test_CompressRGB32ToBinaryEuclidean : public UnitTest{
public:
    Test_CompressRGB32ToBinaryEuclidean(
        const std::string& image
    )
        : UnitTest("Kernels::CompressRGB32ToBinaryEuclidean - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);

        const size_t width = image.width();
        const size_t height = image.height();
        cout << "Testing test_kernels_CompressRGB32ToBinaryEuclidean(), image size " << width << " x " << height << endl;

        Color middle_color = Color(image.pixel(width / 2, height / 2));
        cout << "Expected color: " << middle_color.to_string() << endl;

        double max_dist = 50.0;
        size_t max_dist2 = size_t(max_dist * max_dist);

        PackedBinaryMatrix matrix(image.width(), image.height());

        auto time_start = current_time();
        Kernels::compress_rgb32_to_binary_euclidean(
            image.data(), image.bytes_per_row(), matrix,
            uint32_t(middle_color), max_dist
        );
        auto time_end = current_time();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
        auto ms = ns / 1000000.;
        cout << "One filter time: " << ms << " ms" << endl;

        size_t error_count = 0;
        for (size_t y = 0; y < height; y++){
            for (size_t x = 0; x < width; x++){
                const Color color(image.pixel(x, y));
                int red_dif = (color.red() - middle_color.red());
                int green_dif = (color.green() - middle_color.green());
                int blue_dif = (color.blue() - middle_color.blue());
                size_t color_dist2 = red_dif * red_dif + green_dif * green_dif + blue_dif * blue_dif;
                bool in_range = color_dist2 <= max_dist2;
                if (error_count < 10){
                    // Print first 10 errors:
                    if (in_range && matrix.get(x, y) == false){
                        cout << "Error: wrong filter result: old color " << color.to_string()
                            << ", (x,y) = (" << x << ", " << y << ")"
                            << ", dist2 " << color_dist2 << ", max dist2 " << max_dist2
                            << ", should be in range but not set on matrix" << endl;
                        ++error_count;
                    } else if (in_range == false && matrix.get(x, y) == true){
                        cout << "Error: wrong filter result: old color " << color.to_string()
                            << ", (x,y) = (" << x << ", " << y << ")"
                            << ", dist2 " << color_dist2 << ", max dist2 " << max_dist2
                            << ", should not be in range but set on matrix" << endl;
                        ++error_count;
                    }
                }
            }
        }
        if (error_count){
            return false;
        }

        // We try to wait for three seconds:
        const size_t num_iters = size_t(3000 / ms);
        time_start = current_time();
        for (size_t i = 0; i < num_iters; i++){
            Kernels::compress_rgb32_to_binary_euclidean(
                image.data(), image.bytes_per_row(), matrix,
                uint32_t(middle_color), max_dist
            );
        }
        time_end = current_time();
        ms = (double)std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
        cout << "Running " << num_iters << " iters, avg filter time: " << ms / num_iters << " ms" << endl;

        return true;
    };

private:
    std::string m_image;
};





void add_tests_ImageFilters(UnitTestDatabase& database){



}



}
}
