/*  Image Filters Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <format>
#include <memory>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "Tests/GuardPageImage.h"
#include "Kernels_ImageFilter_Tests.h"

namespace PokemonAutomation{
namespace Kernels{


//  The range filter implementations, defined in RGB32_Range/Kernels_ImageFilter_RGB32_Range_*.cpp.
//  `filter_rgb32_range()` and `to_blackwhite_rgb32_range()` in Kernels_ImageFilter_RGB32_Range.cpp
//  declare the same functions and call the best one for the CPU. These tests call them directly,
//  so that every implementation this CPU supports is tested.
size_t filter_rgb32_range_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t mins, uint32_t maxs
);
size_t to_blackwhite_rgb32_range_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
);
size_t filter_rgb32_range_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t mins, uint32_t maxs
);
size_t to_blackwhite_rgb32_range_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
);
size_t filter_rgb32_range_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t mins, uint32_t maxs
);
size_t to_blackwhite_rgb32_range_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
);
size_t filter_rgb32_range_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t mins, uint32_t maxs
);
size_t to_blackwhite_rgb32_range_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
);
size_t filter_rgb32_range_arm64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t mins, uint32_t maxs
);
size_t to_blackwhite_rgb32_range_arm64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
);

//  The filter_by_mask() implementations, one per binary matrix type, defined in
//  BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Core_*.cpp. `filter_by_mask()` in
//  Kernels_BinaryImage_BasicFilters.cpp declares the same functions and calls the one for
//  the matrix type.
void filter_by_mask_64x4_Default      (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_by_mask_64x8_x64_SSE42    (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_by_mask_64x16_x64_AVX2    (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_by_mask_64x32_x64_AVX512  (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_by_mask_64x64_x64_AVX512  (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);
void filter_by_mask_64x8_arm64_NEON   (const PackedBinaryMatrix_IB& matrix, uint32_t* image, size_t bytes_per_row, uint32_t replace_with, bool replace_if_zero);


namespace{

//  One range filter implementation to test: its name in the test name, and its two functions.
struct RangeFilterImplementation{
    using Filter = size_t (*)(
        const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
        uint32_t* out, size_t out_bytes_per_row,
        uint32_t replacement, bool replace_color_within_range,
        uint32_t mins, uint32_t maxs
    );
    using ToBlackWhite = size_t (*)(
        const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
        uint32_t* out, size_t out_bytes_per_row,
        bool in_range_black,
        uint32_t mins, uint32_t maxs
    );

    std::string name;
    Filter filter;
    ToBlackWhite to_blackwhite;
};

//  One filter_by_mask() implementation to test: its name in the test name, the binary
//  matrix type it works on, and the function.
struct FilterByMaskImplementation{
    using Function = void (*)(
        const PackedBinaryMatrix_IB& matrix,
        uint32_t* image, size_t bytes_per_row,
        uint32_t replace_with, bool replace_if_zero
    );

    std::string name;
    BinaryMatrixType matrix_type;
    Function filter_by_mask;
};


//  An ARGB color range: a pixel is in range if each of its A, R, G and B bytes is within
//  the bytes of `mins` and `maxs`.
struct ColorRange{
    uint32_t mins;
    uint32_t maxs;
};

//  The ranges to test. Each has pixels both in and out of range on every test image. The
//  bounds include 0x00, 0x7f, 0x80 and 0xff, the edges of signed and unsigned bytes.
constexpr ColorRange RANGES[] = {
    {0xff000000, 0xff8080ff},
    {0xff204060, 0xffe0c0ff},
    {0xff60607f, 0xffc8c8c8},
    {0xffa0a0a0, 0xffffffff},
};

constexpr uint32_t REPLACEMENT_COLOR = 0x13572468;
constexpr uint32_t BLACK = 0xff000000;
constexpr uint32_t WHITE = 0xffffffff;

//  Whether `pixel` is in `range`. For example, 0xff7f3080 is in the range
//  [0xff000000, 0xff8080ff], but 0xff813080 is not, since its R (0x81) is above 0x80.
bool in_range(uint32_t pixel, const ColorRange& range){
    for (int shift = 0; shift < 32; shift += 8){
        const uint32_t value = (pixel >> shift) & 0xff;
        if (value < ((range.mins >> shift) & 0xff) || value > ((range.maxs >> shift) & 0xff)){
            return false;
        }
    }
    return true;
}

size_t count_in_range(const GuardPageImageRGB32& image, const ColorRange& range){
    size_t count = 0;
    for (size_t y = 0; y < image.height(); y++){
        for (size_t x = 0; x < image.width(); x++){
            count += in_range(image.pixel(x, y), range);
        }
    }
    return count;
}

//  Compare each pixel of `output` with `expected_pixel(<the input pixel>)`. Return a
//  description of the first wrong pixel, or an empty string if all pixels are right.
template <typename ExpectedPixel>
std::string find_wrong_pixel(
    const GuardPageImageRGB32& input, const GuardPageImageRGB32& output,
    ExpectedPixel expected_pixel
){
    for (size_t y = 0; y < input.height(); y++){
        for (size_t x = 0; x < input.width(); x++){
            const uint32_t pixel = input.pixel(x, y);
            const uint32_t expected = expected_pixel(pixel);
            if (output.pixel(x, y) != expected){
                return std::format(
                    "pixel ({}, {}) is {:08x}: got {:08x} but should be {:08x}",
                    x, y, pixel, output.pixel(x, y), expected
                );
            }
        }
    }
    return "";
}

std::string describe(const ColorRange& range){
    return std::format("range [{:08x}, {:08x}]", range.mins, range.maxs);
}



//  Check one implementation of `filter_rgb32_range()` and `to_blackwhite_rgb32_range()`
//  on a real image, for each of `RANGES`, against `in_range()`:
//  - `filter_rgb32_range()` copies the image, but replaces the pixels in range (or the
//    pixels out of range) with `REPLACEMENT_COLOR`.
//  - `to_blackwhite_rgb32_range()` makes the pixels in range black and the others white
//    (or the opposite).
//  Both must return the number of pixels in range. The input and output are guard-page
//  images, so the test also crashes if the implementation reads or writes past the last
//  pixel.
class Test_ImageFilterRGB32Range : public UnitTest{
public:
    Test_ImageFilterRGB32Range(
        const RangeFilterImplementation& implementation,
        const std::string& image
    )
        : UnitTest("Kernels::ImageFilterRGB32Range - " + implementation.name + " - " + image)
        , m_implementation(implementation)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const ImageRGB32 file_image(m_image);
        const GuardPageImageRGB32 image(file_image);
        GuardPageImageRGB32 output(image.width(), image.height());

        for (const ColorRange& range : RANGES){
            scope.throw_if_cancelled();
            const size_t expected_count = count_in_range(image, range);

            for (bool replace_color_within_range : {true, false}){
                const size_t count = m_implementation.filter(
                    image.data(), image.bytes_per_row(), image.width(), image.height(),
                    output.data(), output.bytes_per_row(),
                    REPLACEMENT_COLOR, replace_color_within_range,
                    range.mins, range.maxs
                );
                const std::string error = find_wrong_pixel(image, output, [&](uint32_t pixel){
                    return in_range(pixel, range) == replace_color_within_range ? REPLACEMENT_COLOR : pixel;
                });
                const std::string name = std::format(
                    "filter_rgb32_range(), {}, replace_color_within_range = {}",
                    describe(range), replace_color_within_range
                );
                if (count != expected_count){
                    return std::format("Error: {}: counted {} pixels in range but should be {}.", name, count, expected_count);
                }
                if (!error.empty()){
                    return std::format("Error: {}: {}.", name, error);
                }
            }

            for (bool in_range_black : {true, false}){
                const size_t count = m_implementation.to_blackwhite(
                    image.data(), image.bytes_per_row(), image.width(), image.height(),
                    output.data(), output.bytes_per_row(),
                    in_range_black,
                    range.mins, range.maxs
                );
                const std::string error = find_wrong_pixel(image, output, [&](uint32_t pixel){
                    return in_range(pixel, range) == in_range_black ? BLACK : WHITE;
                });
                const std::string name = std::format(
                    "to_blackwhite_rgb32_range(), {}, in_range_black = {}",
                    describe(range), in_range_black
                );
                if (count != expected_count){
                    return std::format("Error: {}: counted {} pixels in range but should be {}.", name, count, expected_count);
                }
                if (!error.empty()){
                    return std::format("Error: {}: {}.", name, error);
                }
            }
        }
        return true;
    }

private:
    RangeFilterImplementation m_implementation;
    std::string m_image;
};



//  Check one `filter_by_mask()` implementation on a real image, for each of `RANGES`.
//  The mask is a binary matrix of the implementation's type with a 1 bit for each pixel
//  in range. It is made one bit at a time with `PackedBinaryMatrix_IB::set()`, so it does
//  not depend on the SIMD code of `compress_rgb32_to_binary_range()`. Then
//  `filter_by_mask()` must replace the pixels with 0 bits (or the pixels with 1 bits) with
//  `REPLACEMENT_COLOR`, and keep the other pixels. The image is a guard-page image, so the
//  test also crashes if the implementation reads or writes past the last pixel.
class Test_ImageFilterByMask : public UnitTest{
public:
    Test_ImageFilterByMask(
        const FilterByMaskImplementation& implementation,
        const std::string& image
    )
        : UnitTest("Kernels::ImageFilterByMask - " + implementation.name + " - " + image)
        , m_implementation(implementation)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const ImageRGB32 file_image(m_image);
        const GuardPageImageRGB32 image(file_image);

        for (const ColorRange& range : RANGES){
            scope.throw_if_cancelled();
            std::unique_ptr<PackedBinaryMatrix_IB> mask = make_PackedBinaryMatrix(
                m_implementation.matrix_type, image.width(), image.height()
            );
            for (size_t y = 0; y < image.height(); y++){
                for (size_t x = 0; x < image.width(); x++){
                    mask->set(x, y, in_range(image.pixel(x, y), range));
                }
            }

            for (bool replace_if_zero : {true, false}){
                GuardPageImageRGB32 filtered(file_image);
                m_implementation.filter_by_mask(
                    *mask, filtered.data(), filtered.bytes_per_row(),
                    REPLACEMENT_COLOR, replace_if_zero
                );
                //  The mask bit is 1 if the pixel is in range. The pixel is replaced if
                //  that bit is 0 and `replace_if_zero` is true, or if it is 1 and
                //  `replace_if_zero` is false.
                const std::string error = find_wrong_pixel(image, filtered, [&](uint32_t pixel){
                    return in_range(pixel, range) != replace_if_zero ? REPLACEMENT_COLOR : pixel;
                });
                if (!error.empty()){
                    return std::format(
                        "Error: mask of {}, replace_if_zero = {}: {}.",
                        describe(range), replace_if_zero, error
                    );
                }
            }
        }
        return true;
    }

private:
    FilterByMaskImplementation m_implementation;
    std::string m_image;
};

}



//  Add a test for each image and each range filter and filter_by_mask() implementation
//  that this build has and this CPU supports, including Default.
void add_tests_ImageFilters(UnitTestDatabase& database){
    std::vector<RangeFilterImplementation> range_filters{
        {"Default", filter_rgb32_range_Default, to_blackwhite_rgb32_range_Default},
    };
    std::vector<FilterByMaskImplementation> mask_filters{
        {"64x4_Default", BinaryMatrixType::i64x4_Default, filter_by_mask_64x4_Default},
    };
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        range_filters.push_back({"x64_SSE42", filter_rgb32_range_x64_SSE42, to_blackwhite_rgb32_range_x64_SSE42});
        mask_filters.push_back({"64x8_x64_SSE42", BinaryMatrixType::i64x8_x64_SSE42, filter_by_mask_64x8_x64_SSE42});
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        range_filters.push_back({"x64_AVX2", filter_rgb32_range_x64_AVX2, to_blackwhite_rgb32_range_x64_AVX2});
        mask_filters.push_back({"64x16_x64_AVX2", BinaryMatrixType::i64x16_x64_AVX2, filter_by_mask_64x16_x64_AVX2});
    }
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        range_filters.push_back({"x64_AVX512", filter_rgb32_range_x64_AVX512, to_blackwhite_rgb32_range_x64_AVX512});
        mask_filters.push_back({"64x32_x64_AVX512", BinaryMatrixType::i64x32_x64_AVX512, filter_by_mask_64x32_x64_AVX512});
        mask_filters.push_back({"64x64_x64_AVX512", BinaryMatrixType::i64x64_x64_AVX512, filter_by_mask_64x64_x64_AVX512});
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        range_filters.push_back({"arm64_NEON", filter_rgb32_range_arm64_NEON, to_blackwhite_rgb32_range_arm64_NEON});
        mask_filters.push_back({"64x8_arm64_NEON", BinaryMatrixType::arm64x8_x64_NEON, filter_by_mask_64x8_arm64_NEON});
    }
#endif

    //  Images in the CommandLineTests repo. Their widths cover every remainder mod 4, and
    //  no width or height is a multiple of 8, so the leftover pixels after the SIMD loops
    //  and the binary matrix tiles at the right and bottom edges are tested too.
    const std::vector<std::string> images{
        "NintendoSwitch/Switch2/ghostassassin/system_settings_display_dark.png",    //  1916 x 1078
        "OCR/Lv.1-0.png",                                                           //  237 x 77
        "PokemonSwSh/SelectionArrowFinder/mac_neon_error_1.png",                    //  106 x 410
        "OCR/german-nature-sanft.png",                                              //  511 x 91
    };

    for (const std::string& image : images){
        for (const RangeFilterImplementation& implementation : range_filters){
            database.add<Test_ImageFilterRGB32Range>(implementation, image);
        }
        for (const FilterByMaskImplementation& implementation : mask_filters){
            database.add<Test_ImageFilterByMask>(implementation, image);
        }
    }
}



}
}
