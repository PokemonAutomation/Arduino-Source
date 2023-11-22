/*  Kernels Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic.h"
#include "Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness.h"
#include "Kernels_Tests.h"
#include "TestUtils.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::flush;

namespace PokemonAutomation{

using namespace Kernels;

namespace Kernels{

std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x4_Default(size_t width, size_t height);

void compress_rgb32_to_binary_range_64x4_Default(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix0, uint32_t mins0, uint32_t maxs0
);


}


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
    const size_t width = image.width();
    const size_t height = image.height();

    const uint32_t mins = combine_rgb(0, 0, 0);
    // uint32_t maxs = combine_rgb(255, 255, 255);
    const uint32_t maxs = combine_rgb(63, 63, 63);

    bool have_error = false;

    auto matrix_default = make_PackedBinaryMatrix_64x4_Default(width, height);

    compress_rgb32_to_binary_range_64x4_Default(
        image.data(), image.bytes_per_row(), *matrix_default, mins, maxs
    );

    const size_t num_iter = 3000;

    {
        auto time_start = current_time();
        for(size_t i = 0; i < num_iter; i++){
            compress_rgb32_to_binary_range_64x4_Default(
                image.data(), image.bytes_per_row(), *matrix_default, mins, maxs
            );
        }
        auto time_end = current_time();
        const auto ms = std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
        cout << "Default impl. time: " << ms << " ms" << endl;
    }

    // cout << matrix_default->dump() << flush;

    cout << "Testing current binary matrix construction from image" << endl;
    auto matrix_type = get_BinaryMatrixType();
    auto matrix_current = make_PackedBinaryMatrix(matrix_type, width, height);

    compress_rgb32_to_binary_range(
        image.data(), image.bytes_per_row(), *matrix_current, mins, maxs
    );

    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            const bool v_default = matrix_default->get(c, r);
            const bool v_m1 = matrix_current->get(c, r);
            if (v_default != v_m1){
                cout << "Error: matrix (" << c << ", " << r << ") not same: default: "
                     << v_default << ", M1: " << v_m1 << endl;
                have_error = true;
            }
        }
    }
    if (have_error){
        return 1;
    }

    { // time test
        auto time_start = current_time();
        for(size_t i = 0; i < num_iter; i++){
            compress_rgb32_to_binary_range(
                image.data(), image.bytes_per_row(), *matrix_current, mins, maxs
            );
        }
        auto time_end = current_time();
        const auto ms = std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
        cout << "Cur impl. time: " << ms << " ms" << endl;
    }

    return 0;
}

int test_kernels_FilterRGB32(const ImageViewRGB32& image){
    const size_t width = image.width();
    const size_t height = image.height();
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
        image_out.data(), image_out.bytes_per_row(), mins, maxs, (uint32_t)COLOR_WHITE, replace_color_within_range
    );
    auto time_end = current_time();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
    auto ms = ns / 1000000.;
    cout << "One filter time: " << ms << " ms" << endl;

    size_t pixels_in_range_2 = Kernels::filter_rgb32_range(
        image.data(), image.bytes_per_row(), image.width(), image.height(),
        image_out_2.data(), image_out_2.bytes_per_row(), mins, maxs, (uint32_t)COLOR_WHITE, !replace_color_within_range
    );

    TEST_RESULT_EQUAL(pixels_in_range, pixels_in_range_2);

    size_t actual_num_pixels_in_range = 0;
    size_t error_count = 0;
    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            const Color color(image.pixel(c, r));
            const Color new_color(image_out.pixel(c, r));
            const Color new_color_2(image_out_2.pixel(c, r));
            bool in_range = (min_color.red() <= color.red() && color.red() <= max_color.red());
            in_range = in_range && (min_color.green() <= color.green() && color.green() <= max_color.green());
            in_range = in_range && (min_color.blue() <= color.blue() && color.blue() <= max_color.blue());
            actual_num_pixels_in_range += in_range;
            if (error_count < 10){
                // Print first 10 errors:
                if (in_range && new_color != COLOR_WHITE){
                    cout << "Error: wrong filter result: old color " << color.to_string() << ", (x,y) = "
                        << c << ", " << r << ", should be in range but not found by the function" << endl;
                    ++error_count;
                }
                else if (in_range == false && new_color != color){
                    cout << "Error: wrong filter result: old color " << color.to_string() << ", (x,y) = "
                        << c << ", " << r << ", should not be changed by the function" << endl;
                    ++error_count;
                }

                if (in_range && new_color_2 != color){
                    cout << "Error: wrong inverse filter result: old color " << color.to_string() << ", (x,y) = "
                        << c << ", " << r << ", should not be changed by the function" << endl;
                    ++error_count;
                }
                else if (in_range == false && new_color_2 != COLOR_WHITE){
                    cout << "Error: wrong inverse filter result: old color " << color.to_string() << ", (x,y) = "
                        << c << ", " << r << ", should not be in range but not found by the function" << endl;
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
            image_out.data(), image_out.bytes_per_row(), mins, maxs, (uint32_t)COLOR_WHITE, replace_color_within_range
        );
    }
    time_end = current_time();
    ms = (double)std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
    cout << "Running " << num_iters << " iters, avg filter time: " << ms / num_iters << " ms" << endl;

    return 0;
}

int test_kernels_Waterfill(const ImageViewRGB32& image){

    ImagePixelBox box(0, 0, image.width(), image.height());
    ImageViewRGB32 sub_image = extract_box_reference(image, box);

    PackedBinaryMatrix matrix(sub_image.width(), sub_image.height());
    uint32_t mins = combine_rgb(0, 0, 0);
    // uint32_t maxs = combine_rgb(255, 255, 255);
    uint32_t maxs = combine_rgb(63, 63, 63);
    Kernels::compress_rgb32_to_binary_range(
        sub_image.data(), sub_image.bytes_per_row(),
        matrix, mins, maxs
    );

    cout << matrix.dump() << flush;

    return 0;
}

}
