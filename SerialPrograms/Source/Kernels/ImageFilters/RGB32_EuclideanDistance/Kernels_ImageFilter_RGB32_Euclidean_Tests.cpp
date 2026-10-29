/*  Image Filters RGB32 Euclidean Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Checks every ISA's Euclidean distance filters against a scalar reference.
 *  A pixel is in range when the sum of the squared differences of its R, G
 *  and B to the expected color is <= the squared distance limit (rounded down
 *  to an integer). Alpha is ignored.
 *
 *  Most test pixels sit exactly at, one below, or one above the squared limit,
 *  so an implementation that gets the boundary wrong fails.
 *
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <format>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "Kernels_ImageFilter_RGB32_Euclidean_Tests.h"

namespace PokemonAutomation{
namespace Kernels{



using FilterRgb32Euclidean = size_t (*)(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
);
using ToBlackWhiteRgb32Euclidean = size_t (*)(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t expected, double max_euclidean_distance
);

size_t filter_rgb32_euclidean_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
);
size_t to_blackwhite_rgb32_euclidean_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t expected, double max_euclidean_distance
);
#ifdef PA_AutoDispatch_x64_08_Nehalem
size_t filter_rgb32_euclidean_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
);
size_t to_blackwhite_rgb32_euclidean_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t expected, double max_euclidean_distance
);
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
size_t filter_rgb32_euclidean_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
);
size_t to_blackwhite_rgb32_euclidean_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t expected, double max_euclidean_distance
);
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
size_t filter_rgb32_euclidean_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
);
size_t to_blackwhite_rgb32_euclidean_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t expected, double max_euclidean_distance
);
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
size_t filter_rgb32_euclidean_ARM64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
);
size_t to_blackwhite_rgb32_euclidean_ARM64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t expected, double max_euclidean_distance
);
#endif



namespace{

//  R = 122, G = 128, B = 134: every test offset (at most 120 per channel)
//  stays inside 0..255.
const uint32_t EXPECTED_COLOR = 0xff7a8086;
const int MAX_OFFSET = 120;
const uint32_t REPLACEMENT_COLOR = 0x13572468;

const double MAX_DISTANCES[] = {0.0, 1.0, 50.0, 100.5};

//  Widths around the SIMD vector sizes (4, 8, 16 pixels) and the binary
//  matrix tile width (64 pixels), so the full words and the partial tails
//  both see boundary pixels.
const size_t TEST_SIZES[][2] = {
    {1, 1}, {2, 1}, {3, 2}, {4, 2}, {5, 3}, {7, 2}, {8, 2}, {9, 3},
    {15, 2}, {16, 2}, {17, 3}, {31, 2}, {32, 2}, {33, 2},
    {63, 2}, {64, 2}, {65, 3}, {100, 3}, {128, 2}, {129, 3}, {200, 5},
};

struct Offset{
    int r;
    int g;
    int b;
};

int channel(uint32_t pixel, int shift){
    return (int)((pixel >> shift) & 0xff);
}
uint32_t distance_squared(uint32_t pixel){
    int r = channel(pixel, 16) - channel(EXPECTED_COLOR, 16);
    int g = channel(pixel,  8) - channel(EXPECTED_COLOR,  8);
    int b = channel(pixel,  0) - channel(EXPECTED_COLOR,  0);
    return (uint32_t)(r * r + g * g + b * b);
}

//  Up to "limit" RGB offsets whose squared length is exactly "target".
void add_offsets(std::vector<Offset>& offsets, uint32_t target, size_t limit){
    size_t added = 0;
    for (int r = 0; r <= MAX_OFFSET && (uint32_t)(r * r) <= target; r++){
        for (int g = 0; g <= MAX_OFFSET && (uint32_t)(r * r + g * g) <= target; g++){
            uint32_t left = target - (uint32_t)(r * r + g * g);
            int b = (int)std::lround(std::sqrt((double)left));
            if (b > MAX_OFFSET || (uint32_t)(b * b) != left){
                continue;
            }
            //  Same length with other signs and channel orders.
            offsets.push_back({ r,  g,  b});
            offsets.push_back({-r,  g, -b});
            offsets.push_back({ g, -b,  r});
            offsets.push_back({-b, -r, -g});
            added += 4;
            if (added >= limit){
                return;
            }
        }
    }
}

AlignedVector<uint32_t> make_image(double max_distance, size_t width, size_t height){
    uint32_t limit = (uint32_t)(max_distance * max_distance);

    std::vector<Offset> boundary;
    if (limit > 0){
        add_offsets(boundary, limit - 1, 16);
    }
    add_offsets(boundary, limit, 16);
    add_offsets(boundary, limit + 1, 16);

    int spread = std::min((int)max_distance + 2, MAX_OFFSET);

    std::mt19937 generator((unsigned)(width * 1000003u + height * 7919u + limit));
    std::uniform_int_distribution<size_t> pick(0, boundary.size() - 1);
    std::uniform_int_distribution<int> coin(0, 3);
    std::uniform_int_distribution<int> random_offset(-spread, spread);
    std::uniform_int_distribution<uint32_t> random_alpha(0, 255);

    AlignedVector<uint32_t> image(width * height);
    for (size_t c = 0; c < width * height; c++){
        //  Three in four pixels are boundary cases, the rest are random
        //  colors near the expected one.
        Offset offset = coin(generator) != 0
            ? boundary[pick(generator)]
            : Offset{random_offset(generator), random_offset(generator), random_offset(generator)};
        uint32_t r = (uint32_t)(channel(EXPECTED_COLOR, 16) + offset.r);
        uint32_t g = (uint32_t)(channel(EXPECTED_COLOR,  8) + offset.g);
        uint32_t b = (uint32_t)(channel(EXPECTED_COLOR,  0) + offset.b);
        image[c] = (random_alpha(generator) << 24) | (r << 16) | (g << 8) | b;
    }
    return image;
}

std::string hex(uint32_t x){
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "0x%08x", x);
    return buffer;
}
std::string describe_case(double max_distance, size_t width, size_t height){
    //  std::format uses the same "g" presentation for doubles as %g.
    return std::format("max distance {}, {}x{}", max_distance, width, height);
}
std::string describe_pixel(uint32_t pixel, size_t x, size_t y, uint32_t limit){
    return "pixel " + hex(pixel) + " at (" + std::to_string(x) + ", " + std::to_string(y) +
        "), distance squared " + std::to_string(distance_squared(pixel)) +
        ", limit " + std::to_string(limit);
}



class Test_FilterRgb32Euclidean : public UnitTest{
public:
    Test_FilterRgb32Euclidean(
        const std::string& arch_str,
        FilterRgb32Euclidean filter,
        ToBlackWhiteRgb32Euclidean to_blackwhite
    )
        : UnitTest("Kernels::RGB32Euclidean - filter_" + arch_str)
        , m_filter(filter)
        , m_to_blackwhite(to_blackwhite)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        for (double max_distance : MAX_DISTANCES){
            uint32_t limit = (uint32_t)(max_distance * max_distance);
            for (const auto& size : TEST_SIZES){
                scope.throw_if_cancelled();
                const size_t width = size[0];
                const size_t height = size[1];
                const size_t bytes_per_row = width * sizeof(uint32_t);
                const std::string name = describe_case(max_distance, width, height);

                AlignedVector<uint32_t> in = make_image(max_distance, width, height);
                AlignedVector<uint32_t> out(width * height);

                size_t expected_count = 0;
                for (size_t c = 0; c < width * height; c++){
                    expected_count += distance_squared(in[c]) <= limit;
                }

                for (bool replace_color_within_range : {true, false}){
                    size_t count = m_filter(
                        in.data(), bytes_per_row, width, height,
                        out.data(), bytes_per_row,
                        REPLACEMENT_COLOR, replace_color_within_range,
                        EXPECTED_COLOR, max_distance
                    );
                    if (count != expected_count){
                        return "filter, " + name + ": counted " + std::to_string(count) +
                            " pixels in range, expected " + std::to_string(expected_count);
                    }
                    for (size_t c = 0; c < width * height; c++){
                        bool in_range = distance_squared(in[c]) <= limit;
                        uint32_t expected = in_range == replace_color_within_range ? REPLACEMENT_COLOR : in[c];
                        if (out[c] != expected){
                            return "filter, " + name + ", " + describe_pixel(in[c], c % width, c / width, limit) +
                                ": got " + hex(out[c]) + ", expected " + hex(expected);
                        }
                    }
                }

                for (bool in_range_black : {true, false}){
                    size_t count = m_to_blackwhite(
                        in.data(), bytes_per_row, width, height,
                        out.data(), bytes_per_row,
                        in_range_black,
                        EXPECTED_COLOR, max_distance
                    );
                    if (count != expected_count){
                        return "to_blackwhite, " + name + ": counted " + std::to_string(count) +
                            " pixels in range, expected " + std::to_string(expected_count);
                    }
                    for (size_t c = 0; c < width * height; c++){
                        bool in_range = distance_squared(in[c]) <= limit;
                        uint32_t expected = in_range == in_range_black ? 0xff000000 : 0xffffffff;
                        if (out[c] != expected){
                            return "to_blackwhite, " + name + ", " + describe_pixel(in[c], c % width, c / width, limit) +
                                ": got " + hex(out[c]) + ", expected " + hex(expected);
                        }
                    }
                }
            }
        }
        return true;
    }

private:
    FilterRgb32Euclidean m_filter;
    ToBlackWhiteRgb32Euclidean m_to_blackwhite;
};



class Test_CompressRgb32Euclidean : public UnitTest{
public:
    Test_CompressRgb32Euclidean(const std::string& matrix_str, BinaryMatrixType type)
        : UnitTest("Kernels::RGB32Euclidean - compress_" + matrix_str)
        , m_type(type)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        for (double max_distance : MAX_DISTANCES){
            uint32_t limit = (uint32_t)(max_distance * max_distance);
            for (const auto& size : TEST_SIZES){
                scope.throw_if_cancelled();
                const size_t width = size[0];
                const size_t height = size[1];

                AlignedVector<uint32_t> in = make_image(max_distance, width, height);
                std::unique_ptr<PackedBinaryMatrix_IB> matrix = make_PackedBinaryMatrix(m_type, width, height);
                compress_rgb32_to_binary_euclidean(
                    in.data(), width * sizeof(uint32_t), *matrix,
                    EXPECTED_COLOR, max_distance
                );

                for (size_t y = 0; y < height; y++){
                    for (size_t x = 0; x < width; x++){
                        uint32_t pixel = in[x + y * width];
                        bool expected = distance_squared(pixel) <= limit;
                        if (matrix->get(x, y) != expected){
                            return "compress, " + describe_case(max_distance, width, height) + ", " +
                                describe_pixel(pixel, x, y, limit) +
                                ": got " + (expected ? "out of range" : "in range");
                        }
                    }
                }
            }
        }
        return true;
    }

private:
    BinaryMatrixType m_type;
};

}



void add_tests_ImageFilter_RGB32_Euclidean(UnitTestDatabase& database){
    database.add<Test_FilterRgb32Euclidean>(
        "Default", filter_rgb32_euclidean_Default, to_blackwhite_rgb32_euclidean_Default
    );
    database.add<Test_CompressRgb32Euclidean>("64x4_Default", BinaryMatrixType::i64x4_Default);
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        database.add<Test_FilterRgb32Euclidean>(
            "x64_SSE42", filter_rgb32_euclidean_x64_SSE42, to_blackwhite_rgb32_euclidean_x64_SSE42
        );
        database.add<Test_CompressRgb32Euclidean>("64x8_x64_SSE42", BinaryMatrixType::i64x8_x64_SSE42);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        database.add<Test_FilterRgb32Euclidean>(
            "x64_AVX2", filter_rgb32_euclidean_x64_AVX2, to_blackwhite_rgb32_euclidean_x64_AVX2
        );
        database.add<Test_CompressRgb32Euclidean>("64x16_x64_AVX2", BinaryMatrixType::i64x16_x64_AVX2);
    }
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        database.add<Test_FilterRgb32Euclidean>(
            "x64_AVX512", filter_rgb32_euclidean_x64_AVX512, to_blackwhite_rgb32_euclidean_x64_AVX512
        );
        database.add<Test_CompressRgb32Euclidean>("64x32_x64_AVX512", BinaryMatrixType::i64x32_x64_AVX512);
        database.add<Test_CompressRgb32Euclidean>("64x64_x64_AVX512", BinaryMatrixType::i64x64_x64_AVX512);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        database.add<Test_FilterRgb32Euclidean>(
            "arm64_NEON", filter_rgb32_euclidean_ARM64_NEON, to_blackwhite_rgb32_euclidean_ARM64_NEON
        );
        database.add<Test_CompressRgb32Euclidean>("64x8_arm64_NEON", BinaryMatrixType::arm64x8_x64_NEON);
    }
#endif
}



}
}
