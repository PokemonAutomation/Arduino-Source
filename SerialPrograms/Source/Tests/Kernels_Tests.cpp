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
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness.h"
#include "Kernels_Tests.h"

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

    {
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

    const uint32_t mins = combine_rgb(0, 0, 0);
    // uint32_t maxs = combine_rgb(255, 255, 255);
    const uint32_t maxs = combine_rgb(63, 63, 63);

    bool have_error = false;

    const bool replace_color_within_range = true;
    auto new_image = filter_rgb32_range(image, mins, maxs, COLOR_WHITE, replace_color_within_range);

    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            const Color color(image.pixel(c, r));
            const Color new_color(new_image.pixel(c, r));
            bool in_range = (color.red() <= 63 && color.green() <= 63 && color.blue() <= 63);
            if (in_range && uint32_t(new_color) != uint32_t(COLOR_WHITE)){
                cout << "Error: wrong filter result: old color " << color.to_string() << ", (x,y) = "
                     << c << ", " << r << endl;
                have_error = true;
                return 1;
            }
        }
    }

    // const size_t num_iter = 3000;
    // auto time_start = current_time();
    // for(size_t i = 0; i < num_iter; i++){
    //     filter_rgb32(
    //         matrix, new_image.data(), new_image.bytes_per_row(), uint32_t(COLOR_WHITE), replace_if_zero
    //     );
    // }
    // auto time_end = current_time();
    // const auto ms = std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
    // cout << "Filter time: " << ms << " ms" << endl;



    if (have_error){
        return 1;
    }

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
