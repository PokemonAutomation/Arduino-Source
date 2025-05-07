/*  Kernels Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Compiler.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#ifdef PA_AutoDispatch_arm64_20_M1
    #include "Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64x8_arm64_NEON.h"
    #include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h"
    #include "Kernels/Waterfill/Kernels_Waterfill_Core_64x8_arm64_NEON.h"
#endif
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64xH_Default.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64x4_Default.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64xH_Default.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic.h"
#include "Kernels/ImageFilters/RGB32_Range/Kernels_ImageFilter_RGB32_Range.h"
#include "Kernels/ImageFilters/RGB32_EuclideanDistance/Kernels_ImageFilter_RGB32_Euclidean.h"
#include "Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Core_64xH_Default.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Routines.h"
#include "Kernels_Tests.h"
#include "TestUtils.h"

#include <functional>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::flush;

namespace PokemonAutomation{

using namespace Kernels;

namespace Kernels{}

namespace{



}

int test_binary_matrix_tile();


int test_kernels_ImageScaleBrightness(const ImageViewRGB32& image){
    ImageRGB32 new_image = image.copy();
    
    int num_iterations = 500;
    auto time_start = current_time();
    for(int i = 0; i < num_iterations; i++){
        scale_brightness(new_image.width(), new_image.height(), new_image.data(), new_image.bytes_per_row(), 1.2f, 1.3f, 0.5f);
        // break;
    }
    auto time_end = current_time();
    const auto ms = std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
    cout << "Time: " << ms << " ms, " << ms / 1000. << " s" << endl;

    // new_image.save("./output.png");

    return 0;
}


int test_kernels_BinaryMatrix(const ImageViewRGB32& image){

    if (test_binary_matrix_tile() != 0){
        return 1;
    }

    const size_t width = image.width(), height = image.height();

    const Color min_color(0, 0, 0), max_color(63, 63, 63);
    const uint32_t mins = uint32_t(min_color), maxs = uint32_t(max_color);

    auto binary_matrix = make_PackedBinaryMatrix(get_BinaryMatrixType(), width, height);

    auto time_start = current_time();
    compress_rgb32_to_binary_range(
        image.data(), image.bytes_per_row(), *binary_matrix, mins, maxs
    );
    auto time_end = current_time();
    size_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
    double ms = ns / 1000000.;
    cout << "One binary matrix creation. time: " << ms << " ms" << endl;
    
    size_t error_count = 0;
    for (size_t y = 0; y < height; y++){
        for (size_t x = 0; x < width; x++){
            const Color color(image.pixel(x, y));
            bool in_range = (min_color.alpha() <= color.alpha() && color.alpha() <= max_color.alpha());
            in_range = in_range && (min_color.red() <= color.red() && color.red() <= max_color.red());
            in_range = in_range && (min_color.green() <= color.green() && color.green() <= max_color.green());
            in_range = in_range && (min_color.blue() <= color.blue() && color.blue() <= max_color.blue());

            const bool v_default = binary_matrix->get(x, y);
            
            if (error_count < 10){
                if (v_default != in_range){
                    cout << "Error: matrix (" << x << ", " << y << ") got "
                        << v_default << " but GT is " << in_range << endl;
                    ++error_count;
                }
            }
        }
    }
    if (error_count){
        return 1;
    }


    // We try to wait for three seconds:
    const size_t num_iters = size_t(3000 / ms);
    time_start = current_time();
    for(size_t i = 0; i < num_iters; i++){
        compress_rgb32_to_binary_range(
            image.data(), image.bytes_per_row(), *binary_matrix, mins, maxs
        );
    }
    time_end = current_time();
    ms = (double)std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
    cout << "Running " << num_iters << " iters, average creation impl. time: " << ms / (double)num_iters << " ms" << endl;

    // cout << binary_matrix->dump() << flush;

    return 0;
}

int test_kernels_FilterRGB32Range(const ImageViewRGB32& image){
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

    TEST_RESULT_EQUAL(pixels_in_range, pixels_in_range_2);

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
                }else if (in_range == false && new_color != color){
                    cout << "Error: wrong filter result: old color " << color.to_string() << ", (x,y) = "
                        << x << ", " << y << ", should not be changed by the function" << endl;
                    ++error_count;
                }

                if (in_range && new_color_2 != color){
                    cout << "Error: wrong inverse filter result: old color " << color.to_string() << ", (x,y) = "
                        << x << ", " << y << ", should not be changed by the function" << endl;
                    ++error_count;
                }else if (in_range == false && new_color_2 != COLOR_WHITE){
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
        return 1;
    }

    if (error_count){
        return 1;
    }

    // We try to wait for three seconds:
    const size_t num_iters = size_t(3000 / ms);
    time_start = current_time();
    for(size_t i = 0; i < num_iters; i++){
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

    return 0;
}


int test_kernels_FilterRGB32Euclidean(const ImageViewRGB32& image){
    const size_t width = image.width();
    const size_t height = image.height();
    cout << "Testing test_kernels_FilterRGB32Euclidean(), image size " << width << " x " << height << endl;

    Color middle_color = Color(image.pixel(width/2, height/2));
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

    TEST_RESULT_EQUAL(pixels_in_range, pixels_in_range_2);

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
                }else if (in_range == false && new_color != color){
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
                }else if (in_range == false && new_color_2 != COLOR_WHITE){
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
        return 1;
    }

    if (error_count){
        return 1;
    }

    // We try to wait for three seconds:
    const size_t num_iters = size_t(3000 / ms);
    time_start = current_time();
    for(size_t i = 0; i < num_iters; i++){
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

    return 0;
}

int test_kernels_ToBlackWhiteRGB32Range(const ImageViewRGB32& image){
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

    TEST_RESULT_EQUAL(pixels_in_range, pixels_in_range_2);

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
                }else if (in_range == false && new_color != COLOR_WHITE){
                    cout << "Error: wrong filter result: old color " << color.to_string() << ", (x,y) = "
                        << x << ", " << y << ", should be white due to out of range but not so" << endl;
                    ++error_count;
                }

                if (in_range && new_color_2 != COLOR_WHITE){
                    cout << "Error: wrong inverse filter result: old color " << color.to_string() << ", (x,y) = "
                        << x << ", " << y << ", should be white due to in range but not so" << endl;
                    ++error_count;
                }else if (in_range == false && new_color_2 != COLOR_BLACK){
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
        return 1;
    }

    if (error_count){
        return 1;
    }

    // We try to wait for three seconds:
    const size_t num_iters = size_t(3000 / ms);
    time_start = current_time();
    for(size_t i = 0; i < num_iters; i++){
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

    return 0;
}

int test_kernels_FilterByMask(const ImageViewRGB32& image){
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
                }else if (!in_range && new_color != replacement_color){
                    cout << "Error: wrong filter(replace_zero_bits) result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should be changed due to being zero bit but not so" << endl;
                    ++error_count;
                }

                if (in_range && new_color_2 != replacement_color){
                    cout << "Error: wrong filter(replace_one_bits) result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should be changed due to being one bit but not so" << endl;
                    ++error_count;
                }else if (!in_range && new_color_2 != color){
                    cout << "Error: wrong filter(replace_one_bits) result: old color " << color.to_string() << ", (x,y) = "
                            << x << ", " << y << ", should not be changed due to being zero bit but not so" << endl;
                    ++error_count;
                }
            }
        }
    }

    if (error_count){
        return 1;
    }

    // We try to wait for three seconds:
    const size_t num_iters = size_t(3000 / ms);
    time_start = current_time();
    for(size_t i = 0; i < num_iters; i++){
        filter_by_mask(*binary_matrix, new_image.data(), new_image.bytes_per_row(), uint32_t(replacement_color), replace_zero_bits);
    }
    time_end = current_time();
    ms = (double)std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
    cout << "Running " << num_iters << " iters, avg filter time: " << ms / num_iters << " ms" << endl;

    return 0;
}

int test_kernels_CompressRGB32ToBinaryEuclidean(const ImageViewRGB32& image){
    const size_t width = image.width();
    const size_t height = image.height();
    cout << "Testing test_kernels_CompressRGB32ToBinaryEuclidean(), image size " << width << " x " << height << endl;

    Color middle_color = Color(image.pixel(width/2, height/2));
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
                }else if (in_range == false && matrix.get(x, y) == true){
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
        return 1;
    }

    // We try to wait for three seconds:
    const size_t num_iters = size_t(3000 / ms);
    time_start = current_time();
    for(size_t i = 0; i < num_iters; i++){
        Kernels::compress_rgb32_to_binary_euclidean(
            image.data(), image.bytes_per_row(), matrix,
            uint32_t(middle_color), max_dist
        );
    }
    time_end = current_time();
    ms = (double)std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
    cout << "Running " << num_iters << " iters, avg filter time: " << ms / num_iters << " ms" << endl;

    return 0;
}




int test_kernels_Waterfill(const ImageViewRGB32& image){
    const size_t width = image.width();
    const size_t height = image.height();
    cout << "Testing test_kernels_Waterfill(), image size " << width << " x " << height << endl;

    PackedBinaryMatrix matrix(width, height);
    uint32_t mins = combine_rgb(0, 0, 0);
    // uint32_t maxs = combine_rgb(255, 255, 255);
    uint32_t maxs = combine_rgb(63, 63, 63);
    Kernels::compress_rgb32_to_binary_range(
        image.data(), image.bytes_per_row(),
        matrix, mins, maxs
    );

    PackedBinaryMatrix source_matrix = matrix.copy();

    PackedBinaryMatrix gt_matrix = matrix.copy();
    Kernels::PackedBinaryMatrix_IB& gt_matrix_ib = gt_matrix;

    size_t min_area = 10;
    std::vector<Kernels::Waterfill::WaterfillObject> gt_objects;
    bool gt_computed = false;

#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        using Waterfill_64x8_Default = Kernels::Waterfill::Waterfill_64xH_Default<Kernels::BinaryTile_64x8_arm64_NEON>;
        gt_objects = Kernels::Waterfill::find_objects_inplace<Kernels::BinaryTile_64x8_arm64_NEON, Waterfill_64x8_Default>(
            static_cast<Kernels::PackedBinaryMatrix_64x8_arm64_NEON&>(gt_matrix_ib).get(),
            min_area
        );
        gt_computed = true;
    }
#endif
    if (gt_computed == false){
        using Waterfill_64x4_Default = Kernels::Waterfill::Waterfill_64xH_Default<Kernels::BinaryTile_64x4_Default>;
        gt_objects = Kernels::Waterfill::find_objects_inplace<Kernels::BinaryTile_64x4_Default, Waterfill_64x4_Default>(
            static_cast<Kernels::PackedBinaryMatrix_64x4_Default&>(gt_matrix_ib).get(),
            min_area
        );
    }
    cout << "num objects: " << gt_objects.size() << endl;

    auto time_start = current_time();
    std::vector<Kernels::Waterfill::WaterfillObject> objects = Kernels::Waterfill::find_objects_inplace(matrix, min_area);
    auto time_end = current_time();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
    auto ms = ns / 1000000.;
    cout << "One waterfill time: " << ms << " ms" << endl;

    for(size_t i = 0; i < objects.size(); ++i){
        TEST_RESULT_COMPONENT_EQUAL(objects[i].area, gt_objects[i].area, "object " + std::to_string(i) + " area");
        TEST_RESULT_COMPONENT_EQUAL(objects[i].min_x, gt_objects[i].min_x, "object " + std::to_string(i) + " min_x");
        TEST_RESULT_COMPONENT_EQUAL(objects[i].min_y, gt_objects[i].min_y, "object " + std::to_string(i) + " min_y");
        TEST_RESULT_COMPONENT_EQUAL(objects[i].max_x, gt_objects[i].max_x, "object " + std::to_string(i) + " max_x");
        TEST_RESULT_COMPONENT_EQUAL(objects[i].max_y, gt_objects[i].max_y, "object " + std::to_string(i) + " max_y");
    }

    // We try to wait for three seconds:
    const size_t num_iters = size_t(3000 / ms);
    time_start = current_time();
    for(size_t i = 0; i < num_iters; i++){
        matrix = source_matrix.copy();
        objects = Kernels::Waterfill::find_objects_inplace(matrix, min_area);
    }
    time_end = current_time();
    ms = (double)std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
    cout << "Running " << num_iters << " iters, avg filter time: " << ms / num_iters << " ms" << endl;



    return 0;
}

// Additional tests on binary matrix tile implementation
template<class Tile> int test_binary_matrix_tile_t(){
    size_t num_iters = 100000;
    size_t sum = 0;
    WallClock time_start, time_end;
    size_t ns = 0;
    double ms = 0;

    Tile tile;
    // BinaryTile_64x8_arm64_NEON tile;
    // BinaryTile_64xH_Default<8, BinaryMatrixType::arm64x8_x64_NEON> tile;
    for(size_t height = 0; height < tile.HEIGHT; ++height){
        for(size_t width = 0; width < tile.WIDTH; ++width){
            tile.set_zero();
            tile.set_ones(width, height);
            for(size_t y = 0; y < tile.HEIGHT; ++y){
                for(size_t x = 0; x < tile.WIDTH; ++x){
                    bool gt = y < height && x < width;
                    if (gt != tile.get_bit(x, y)){
                        cout << "Tile::set_ones(width = " << width << ", height = " << height << "), tile (x = " << 
                            x << ", y = " << y << "), wrong bit. Should be " << gt << endl;
                        return 1;
                    }
                }
            }
        }
    }

    sum = 0;
    time_start = current_time();
    for(size_t i = 0; i < num_iters; ++i){
        tile.set_ones(32, 4);
        sum += tile.row(0) + tile.row(1) + tile.row(2) + tile.row(3);
    }
    time_end = current_time();
    ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
    ms = ns / 1000000.;
    cout << "Execution enforcer: " << sum << endl;
    cout << "Running " << num_iters << " iters, Tile::set_ones() time: " << ms / num_iters << " ms" << endl;

    for(size_t height = 0; height < tile.HEIGHT; ++height){
        for(size_t width = 0; width < tile.WIDTH; ++width){
            tile.set_ones();
            tile.clear_padding(width, height);
            for(size_t y = 0; y < tile.HEIGHT; ++y){
                for(size_t x = 0; x < tile.WIDTH; ++x){
                    bool gt = y < height && x < width;
                    if (gt != tile.get_bit(x, y)){
                        cout << "Tile::clear_padding(width = " << width << ", height = " << height << "), tile (x = " << 
                            x << ", y = " << y << "), wrong bit. Should be " << gt << endl;
                        return 1;
                    }
                }
            }
        }
    }
    sum = 0;
    time_start = current_time();
    for(size_t i = 0; i < num_iters; ++i){
        tile.clear_padding(32, 4);
        sum += tile.row(0) + tile.row(1) + tile.row(2) + tile.row(3);
    }
    time_end = current_time();
    ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
    ms = ns / 1000000.;
    cout << "Execution enforcer: " << sum << endl;
    cout << "Running " << num_iters << " iters, Tile::clear_padding() time: " << ms / num_iters << " ms" << endl;

    tile.set_ones(35, 3);
    tile.invert();
    for(size_t y = 0; y < tile.HEIGHT; ++y){
        for(size_t x = 0; x < tile.WIDTH; ++x){
            bool gt = !(x < 35 && y < 3);
            if (gt != tile.get_bit(x, y)){
                cout << "Tile::invert(), tile (x = " << 
                     x << ", y = " << y << "), wrong bit. Should be " << gt << endl;
                return 1;
            }
        }
    }

    auto tile2 = tile;
    tile.set_ones(35, 3);
    tile2.set_ones(13, 6);
    tile.andnot(tile2);
    for(size_t y = 0; y < tile.HEIGHT; ++y){
        for(size_t x = 0; x < tile.WIDTH; ++x){
            bool gt = !(x < 13 && y < 6) && (x < 35 && y < 3);
            if (gt != tile.get_bit(x, y)){
                cout << "Tile::andnot(), tile (x = " << 
                    x << ", y = " << y << "), wrong bit. Should be " << gt << endl;
                return 1;
            }
        }
    }

    tile.set_ones(35, 3);
    {
        uint64_t top_row = tile.top();
        for(size_t x = 0; x < tile.WIDTH; ++x){
            uint64_t bit = ((top_row >> x) & 1);
            uint64_t gt = uint64_t(x < 35);
            if (bit != gt){
                cout << "Tile::top(), tile (x = " << 
                    x << "), wrong bit. Should be " << gt << endl;
                return 1;
            }
        }
    }
    {
        tile.set_zero();
        uint64_t& top_row = tile.top();
        top_row = 0xFFFFFFFFFFFFFFFF;
        uint64_t top = tile.top();
        for(size_t x = 0; x < tile.WIDTH; ++x){
            uint64_t bit = ((top >> x) & 1);
            if (bit != 1){
                cout << "& Tile::top(), tile (x = " << 
                    x << "), wrong bit. Should be 1" << endl;
                return 1;
            }
        }
    }
    {
        tile.set_zero();
        uint64_t& bottom_row = tile.bottom();
        bottom_row = 0xFFFFFFFFFFFFFFFF;
        uint64_t bottom = tile.bottom();
        for(size_t x = 0; x < tile.WIDTH; ++x){
            uint64_t bit = ((bottom >> x) & 1);
            if (bit != 1){
                cout << "& Tile::bottom(), tile (x = " << 
                    x << "), wrong bit. Should be 1" << endl;
                return 1;
            }
        }
    }
    tile.set_zero();
    for(size_t y = 0; y < tile.HEIGHT; ++y){
        for(size_t x = 0; x < tile.WIDTH; ++x){
            tile.set_bit(x, y);
            if (1 != tile.get_bit(x, y)){
                cout << "Tile::set_bit(), tile (x = " << 
                     x << ", y = " << y << "), wrong bit. Should be 1" << endl;
                return 1;
            }
            tile.set_bit(x, y, 1);
            if (1 != tile.get_bit(x, y)){
                cout << "Tile::set_bit(), tile (x = " << 
                     x << ", y = " << y << "), wrong bit. Should be 1" << endl;
                return 1;
            }
            tile.set_bit(x, y, 0);
            if (0 != tile.get_bit(x, y)){
                cout << "Tile::set_bit(), tile (x = " << 
                     x << ", y = " << y << "), wrong bit. Should be 0" << endl;
                return 1;
            }
        }
    }

    std::srand(0);
    auto src_tile = tile;
    auto dst_tile = tile;
    for(size_t num_tests = 0; num_tests < 30; ++num_tests){
        src_tile.set_zero();
        dst_tile.set_zero();
        for(size_t y = 0; y < tile.HEIGHT; ++y){
            for(size_t x = 0; x < tile.WIDTH; ++x){
                src_tile.set_bit(x, y, std::rand() % 2);
                dst_tile.set_bit(x, y, std::rand() % 2);
            }
        }

        auto test_copy_to_shift = [&](size_t shift_x, size_t shift_y, const std::string& function_name,
            std::function<bool(size_t, size_t, size_t, size_t)> gt_test,
            std::function<bool(size_t, size_t, size_t, size_t)> get_src_bit_test){
            // check against GT
            for(size_t y = 0; y < tile.HEIGHT; ++y){
                for(size_t x = 0; x < tile.WIDTH; ++x){
                    bool gt = false;
                    if (gt_test(shift_x, shift_y, x, y)){
                        gt = dst_tile.get_bit(x, y) || get_src_bit_test(shift_x, shift_y, x, y);
                        // cout << "set the source bit with ||" << endl;
                    }else{
                        // cout << "Use the original dest bit" << endl;
                        gt = dst_tile.get_bit(x, y);
                    }
                    if (tile.get_bit(x, y) != gt){
                        cout << "Tile::" << function_name << "(shift_x=" << shift_x << ", shift_y=" << shift_y << "), tile (x = " << 
                            x << ", y = " << y << "), wrong bit. Should be " << gt << endl;
                        cout << src_tile.dump() << endl;
                        cout << dst_tile.dump() << endl;
                        cout << tile.dump() << endl;
                        return 1;
                    }
                }
            }
            return 0;
        };

        for(size_t shift_y = 0; shift_y < tile.HEIGHT; ++shift_y){
            for(size_t shift_x = 0; shift_x < tile.WIDTH; ++shift_x){
                tile = dst_tile;
                src_tile.copy_to_shift_pp(tile, shift_x, shift_y);
                auto gt_test_pp = [&](size_t shift_x, size_t shift_y, size_t x, size_t y) -> bool{
                    return x + shift_x < tile.WIDTH && y + shift_y < tile.HEIGHT;
                };
                auto get_src_bit_test_pp = [&](size_t shift_x, size_t shift_y, size_t x, size_t y){
                    return src_tile.get_bit(x + shift_x, y + shift_y);
                };
                if (test_copy_to_shift(shift_x, shift_y, "copy_to_shift_pp", gt_test_pp, get_src_bit_test_pp) != 0){
                    return 1;
                }
                
                tile = dst_tile;
                src_tile.copy_to_shift_np(tile, shift_x, shift_y);
                auto gt_test_np = [&](size_t shift_x, size_t shift_y, size_t x, size_t y) -> bool{
                    return x >= shift_x && y + shift_y < tile.HEIGHT;
                };
                auto get_src_bit_test_np = [&](size_t shift_x, size_t shift_y, size_t x, size_t y){
                    return src_tile.get_bit(x - shift_x, y + shift_y);
                };
                if (test_copy_to_shift(shift_x, shift_y, "copy_to_shift_np", gt_test_np, get_src_bit_test_np) != 0){
                    return 1;
                }

                tile = dst_tile;
                src_tile.copy_to_shift_pn(tile, shift_x, shift_y);
                auto gt_test_pn = [&](size_t shift_x, size_t shift_y, size_t x, size_t y) -> bool{
                    return x + shift_x < tile.WIDTH && y >= shift_y;
                };
                auto get_src_bit_test_pn = [&](size_t shift_x, size_t shift_y, size_t x, size_t y){
                    return src_tile.get_bit(x + shift_x, y - shift_y);
                };
                if (test_copy_to_shift(shift_x, shift_y, "copy_to_shift_pn", gt_test_pn, get_src_bit_test_pn) != 0){
                    return 1;
                }

                tile = dst_tile;
                src_tile.copy_to_shift_nn(tile, shift_x, shift_y);
                auto gt_test_nn = [&](size_t shift_x, size_t shift_y, size_t x, size_t y) -> bool{
                    return x >= shift_x && y >= shift_y;
                };
                auto get_src_bit_test_nn = [&](size_t shift_x, size_t shift_y, size_t x, size_t y){
                    return src_tile.get_bit(x - shift_x, y - shift_y);
                };
                if (test_copy_to_shift(shift_x, shift_y, "copy_to_shift_nn", gt_test_nn, get_src_bit_test_nn) != 0){
                    return 1;
                }
            }
        }
    }

    cout << "Tile tests passed" << endl;
    return 0;
}

int test_binary_matrix_tile(){
#ifdef PA_AutoDispatch_arm64_20_M1
    if (test_binary_matrix_tile_t<BinaryTile_64x8_arm64_NEON>() != 0){
        return 1;
    }

    for(size_t num_bytes = 0; num_bytes <= 16; ++num_bytes){
        PartialWordAccess_arm64_NEON partial(num_bytes);

        uint8_t buffer[48];
        for(int i = 0; i < 48; i++){
            buffer[i] = 99;
        }
        for(uint8_t i = 0; i < 16; i++){
            buffer[i+16] = i;
        }

        // uint8x16_t x = partial.load(buffer+16);
        uint8x16_t x = partial.load_int_no_read_past_end(buffer+16);
        for(size_t i = 0; i < num_bytes; ++i){
            if (x[i] != buffer[i+16]){
                cout << "Error: PartialWordAccess_arm64_NEON(" << num_bytes << ")::load_int_no_read_past_end(), i = " << i << " is " << int(x[i])
                    << ", but should be " << int(buffer[i+16]) << endl;
                return 1;
            }
        }
        x = partial.load_int_no_read_before_ptr(buffer+16);
        for(size_t i = 0; i < num_bytes; ++i){
            if (x[i] != buffer[i+16]){
                cout << "Error: PartialWordAccess_arm64_NEON(" << num_bytes << ")::load_int_no_read_before_ptr(), i = " << i << " is " << int(x[i])
                    << ", but should be " << int(buffer[i+16]) << endl;
                return 1;
            }
        }
        for(int i = 0; i < 48; i++){
            buffer[i] = 99;
        }
        partial.store_int_no_past_end(buffer+16, x);
        for(size_t i = 0; i < num_bytes; ++i){
            if (x[i] != buffer[i+16]){
                cout << "Error: PartialWordAccess_arm64_NEON(" << num_bytes << ")::store_int_no_past_end(), i = " << i << " is " << int(buffer[i+16])
                    << ", but should be " << int(x[i+16]) << endl;
                return 1;
            }
        }
    }
#endif
    return 0;
}

}
