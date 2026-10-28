/*  BinaryMatrix Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <format>
#include <iterator>
#include <memory>
#include <random>
#include <vector>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Kernels_BinaryMatrixTile_64x4_Default.h"
#ifdef PA_AutoDispatch_arm64_20_M1
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h"
#include "Kernels_BinaryMatrixTile_64x8_arm64_NEON.h"
#endif
#include "Tests/GuardPageImage.h"
#include "Kernels_BinaryMatrix_Tests.h"

namespace PokemonAutomation{
namespace Kernels{


namespace{

//  Tile tests: check one binary matrix tile class against `get_bit()`. Only the tiles
//  that compile with this file's default compiler flags are tested here: Default, and NEON
//  on arm64. The x86 tiles need AVX2 or AVX512 flags. They are tested through the matrix
//  tests below and the Waterfill tests.

template <typename Tile>
Tile random_tile(std::mt19937& generator){
    Tile tile;
    for (size_t y = 0; y < Tile::HEIGHT; y++){
        for (size_t x = 0; x < Tile::WIDTH; x++){
            tile.set_bit(x, y, generator() & 1);
        }
    }
    return tile;
}

//  Compare each bit of `tile` with `expected_bit(x, y)`. Return a description of the first
//  wrong bit, or an empty string if all bits are right.
template <typename Tile, typename ExpectedBit>
std::string find_wrong_tile_bit(const Tile& tile, ExpectedBit expected_bit){
    for (size_t y = 0; y < Tile::HEIGHT; y++){
        for (size_t x = 0; x < Tile::WIDTH; x++){
            const bool expected = expected_bit(x, y);
            if (tile.get_bit(x, y) != expected){
                return std::format("tile bit ({}, {}) should be {}", x, y, (int)expected);
            }
        }
    }
    return "";
}

//  `set_ones(width, height)` sets the bits (x, y) with x < width and y < height, and clears
//  the others. `clear_padding(width, height)` clears the other bits and keeps these.
template <typename Tile>
std::string check_tile_masks(std::mt19937& generator){
    for (size_t height = 0; height <= Tile::HEIGHT; height++){
        for (size_t width = 0; width <= Tile::WIDTH; width++){
            auto inside = [=](size_t x, size_t y){
                return x < width && y < height;
            };

            Tile tile = random_tile<Tile>(generator);
            tile.set_ones(width, height);
            std::string error = find_wrong_tile_bit(tile, inside);
            if (!error.empty()){
                return std::format("set_ones({}, {}): {}", width, height, error);
            }

            const Tile original = random_tile<Tile>(generator);
            tile = original;
            tile.clear_padding(width, height);
            error = find_wrong_tile_bit(tile, [&](size_t x, size_t y){
                return inside(x, y) && original.get_bit(x, y);
            });
            if (!error.empty()){
                return std::format("clear_padding({}, {}): {}", width, height, error);
            }
        }
    }
    return "";
}

//  `invert()`, `^=`, `|=`, `&=` and `andnot()` on random tiles. `a.andnot(b)` keeps the
//  bits of `a` that are not set in `b`.
template <typename Tile>
std::string check_tile_bitwise(std::mt19937& generator){
    const Tile a = random_tile<Tile>(generator);
    const Tile b = random_tile<Tile>(generator);

    Tile tile = a;
    tile.invert();
    std::string error = find_wrong_tile_bit(tile, [&](size_t x, size_t y){ return !a.get_bit(x, y); });
    if (!error.empty()){
        return "invert(): " + error;
    }
    tile = a;
    tile ^= b;
    error = find_wrong_tile_bit(tile, [&](size_t x, size_t y){ return a.get_bit(x, y) != b.get_bit(x, y); });
    if (!error.empty()){
        return "operator^=(): " + error;
    }
    tile = a;
    tile |= b;
    error = find_wrong_tile_bit(tile, [&](size_t x, size_t y){ return a.get_bit(x, y) || b.get_bit(x, y); });
    if (!error.empty()){
        return "operator|=(): " + error;
    }
    tile = a;
    tile &= b;
    error = find_wrong_tile_bit(tile, [&](size_t x, size_t y){ return a.get_bit(x, y) && b.get_bit(x, y); });
    if (!error.empty()){
        return "operator&=(): " + error;
    }
    tile = a;
    tile.andnot(b);
    error = find_wrong_tile_bit(tile, [&](size_t x, size_t y){ return a.get_bit(x, y) && !b.get_bit(x, y); });
    if (!error.empty()){
        return "andnot(): " + error;
    }
    return "";
}

//  `row(y)` is the 64-bit word of row y, with bit x of the word at (x, y). `top()` is row 0
//  and `bottom()` is the last row. `set_bit()` changes only one bit.
template <typename Tile>
std::string check_tile_rows(std::mt19937& generator){
    std::vector<uint64_t> words(Tile::HEIGHT);
    Tile tile;
    for (size_t y = 0; y < Tile::HEIGHT; y++){
        words[y] = ((uint64_t)generator() << 32) | generator();
        tile.row(y) = words[y];
    }
    std::string error = find_wrong_tile_bit(tile, [&](size_t x, size_t y){ return (words[y] >> x) & 1; });
    if (!error.empty()){
        return "row(): " + error;
    }
    if (tile.top() != words[0] || tile.bottom() != words[Tile::HEIGHT - 1]){
        return "top() or bottom() is not the first or last row";
    }
    tile.top() = words[0] = ~words[0];
    tile.bottom() = words[Tile::HEIGHT - 1] = ~words[Tile::HEIGHT - 1];
    error = find_wrong_tile_bit(tile, [&](size_t x, size_t y){ return (words[y] >> x) & 1; });
    if (!error.empty()){
        return "writing top() or bottom(): " + error;
    }

    for (size_t y = 0; y < Tile::HEIGHT; y++){
        for (size_t x = 0; x < Tile::WIDTH; x++){
            const Tile original = tile;
            const bool value = !original.get_bit(x, y);
            tile.set_bit(x, y, value);
            error = find_wrong_tile_bit(tile, [&](size_t bit_x, size_t bit_y){
                return bit_x == x && bit_y == y ? value : original.get_bit(bit_x, bit_y);
            });
            if (!error.empty()){
                return std::format("set_bit({}, {}, {}): {}", x, y, (int)value, error);
            }
        }
    }
    return "";
}

//  One of the `copy_to_shift_*()` functions, which OR a shifted copy of a tile into another
//  tile. They are used by `submatrix()` to build each output tile from the (up to 4) input
//  tiles that it overlaps. The first letter of the suffix is for x and the second for y:
//  "p" means that destination bit x gets source bit x + shift, and "n" means that
//  destination bit x gets source bit x - shift.
template <typename Tile>
struct CopyToShift{
    const char* name;
    void (Tile::*function)(Tile& tile, size_t shift_x, size_t shift_y) const;
    bool positive_x;
    bool positive_y;

    //  The source bit that lands on destination bit (x, y), or false if none does. Going
    //  below 0 wraps around to a huge size_t, which is out of the tile too.
    bool source_bit(const Tile& source, size_t shift_x, size_t shift_y, size_t x, size_t y) const{
        const size_t source_x = positive_x ? x + shift_x : x - shift_x;
        const size_t source_y = positive_y ? y + shift_y : y - shift_y;
        return source_x < Tile::WIDTH && source_y < Tile::HEIGHT && source.get_bit(source_x, source_y);
    }
};

template <typename Tile>
std::string check_tile_shifts(std::mt19937& generator){
    const CopyToShift<Tile> shifts[] = {
        {"copy_to_shift_pp", &Tile::copy_to_shift_pp, true,  true},
        {"copy_to_shift_np", &Tile::copy_to_shift_np, false, true},
        {"copy_to_shift_pn", &Tile::copy_to_shift_pn, true,  false},
        {"copy_to_shift_nn", &Tile::copy_to_shift_nn, false, false},
    };
    const Tile source = random_tile<Tile>(generator);
    const Tile destination = random_tile<Tile>(generator);
    for (const CopyToShift<Tile>& shift : shifts){
        for (size_t shift_y = 0; shift_y < Tile::HEIGHT; shift_y++){
            for (size_t shift_x = 0; shift_x < Tile::WIDTH; shift_x++){
                Tile tile = destination;
                (source.*shift.function)(tile, shift_x, shift_y);
                const std::string error = find_wrong_tile_bit(tile, [&](size_t x, size_t y){
                    return destination.get_bit(x, y) || shift.source_bit(source, shift_x, shift_y, x, y);
                });
                if (!error.empty()){
                    return std::format("{}(shift_x = {}, shift_y = {}): {}", shift.name, shift_x, shift_y, error);
                }
            }
        }
    }
    return "";
}

template <typename Tile>
class Test_BinaryTile : public UnitTest{
public:
    Test_BinaryTile(const std::string& tile_name)
        : UnitTest("Kernels::BinaryMatrix - tile - " + tile_name)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        std::mt19937 generator(0);
        for (auto check : {check_tile_masks<Tile>, check_tile_bitwise<Tile>, check_tile_rows<Tile>, check_tile_shifts<Tile>}){
            const std::string error = check(generator);
            if (!error.empty()){
                return "Error: " + error + ".";
            }
        }
        return true;
    }
};



#ifdef PA_AutoDispatch_arm64_20_M1
//  Check `PartialWordAccess_arm64_NEON`, which loads and stores only the first 0 to 16
//  bytes of a 16-byte NEON vector. NEON kernels use it for the leftover pixels at the end
//  of a row. The test data sits in the middle of a buffer filled with `UNTOUCHED`:
//  - Both loads must return the data bytes in the low lanes and 0 in the other lanes.
//  - The store must write the data bytes and keep every other byte of the buffer.
class Test_PartialWordAccess_arm64_NEON : public UnitTest{
public:
    Test_PartialWordAccess_arm64_NEON()
        : UnitTest("Kernels::PartialWordAccess - arm64_NEON")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        constexpr uint8_t UNTOUCHED = 99;
        constexpr size_t START = 16;

        for (size_t bytes = 0; bytes <= 16; bytes++){
            const PartialWordAccess_arm64_NEON access(bytes);

            uint8_t buffer[48];
            for (size_t i = 0; i < 48; i++){
                buffer[i] = i < START || i >= START + bytes ? UNTOUCHED : (uint8_t)(100 + i);
            }
            const uint8x16_t loads[] = {
                access.load_int_no_read_past_end(buffer + START),
                access.load_int_no_read_before_ptr(buffer + START),
            };
            for (const uint8x16_t& loaded : loads){
                uint8_t lanes[16];
                vst1q_u8(lanes, loaded);
                for (size_t i = 0; i < 16; i++){
                    const int expected = i < bytes ? buffer[START + i] : 0;
                    if (lanes[i] != expected){
                        return std::format("Error: loading {} bytes: lane {} is {} but should be {}.", bytes, i, (int)lanes[i], expected);
                    }
                }
            }

            uint8_t stored[48];
            for (size_t i = 0; i < 48; i++){
                stored[i] = UNTOUCHED;
            }
            access.store_int_no_past_end(stored + START, vld1q_u8(buffer + START));
            for (size_t i = 0; i < 48; i++){
                if (stored[i] != buffer[i]){
                    return std::format("Error: storing {} bytes: byte {} is {} but should be {}.", bytes, i, (int)stored[i], (int)buffer[i]);
                }
            }
        }
        return true;
    }
};
#endif



//  Matrix tests: check every matrix operation that has code per matrix type, through the
//  `PackedBinaryMatrix_IB` interface, against the pixels of a real image.

//  An ARGB color range: a pixel is in range if each of its A, R, G and B bytes is within
//  the bytes of `mins` and `maxs`.
struct ColorRange{
    uint32_t mins;
    uint32_t maxs;
};

//  The ranges to test. Each has pixels both in and out of range on every test image.
constexpr ColorRange RANGES[] = {
    {0xff000000, 0xff8080ff},
    {0xff204060, 0xffe0c0ff},
    {0xff60607f, 0xffc8c8c8},
    {0xffa0a0a0, 0xffffffff},
};

bool in_range(uint32_t pixel, const ColorRange& range){
    for (int shift = 0; shift < 32; shift += 8){
        const uint32_t value = (pixel >> shift) & 0xff;
        if (value < ((range.mins >> shift) & 0xff) || value > ((range.maxs >> shift) & 0xff)){
            return false;
        }
    }
    return true;
}

//  Compare `matrix` with `expected_bit(x, y)`: its size must be `width` x `height`, each bit
//  must match, and each padding bit must be 0. The padding bits are the bits of the edge
//  tiles past the right and bottom of the matrix. They must stay 0, since Waterfill and
//  other code read whole tiles. Return a description of the first problem, or an empty
//  string if there is none.
template <typename ExpectedBit>
std::string find_wrong_bit(
    const PackedBinaryMatrix_IB& matrix, size_t width, size_t height,
    ExpectedBit expected_bit
){
    if (matrix.width() != width || matrix.height() != height){
        return std::format("size is {} x {} but should be {} x {}", matrix.width(), matrix.height(), width, height);
    }
    for (size_t y = 0; y < height; y++){
        for (size_t x = 0; x < width; x++){
            const bool expected = expected_bit(x, y);
            if (matrix.get(x, y) != expected){
                return std::format("bit ({}, {}) should be {}", x, y, (int)expected);
            }
        }
    }

    //  `dump_tiles()` prints every bit of every tile, padding included, as rows of '0' and
    //  '1' that end with '\n'.
    size_t x = 0, y = 0;
    for (char bit : matrix.dump_tiles()){
        if (bit == '\n'){
            x = 0;
            y++;
            continue;
        }
        if (bit == '1' && (x >= width || y >= height)){
            return std::format("padding bit ({}, {}) is 1", x, y);
        }
        x++;
    }
    return "";
}


//  A rectangle to cut out with `submatrix()`.
struct SubmatrixBox{
    size_t x;
    size_t y;
    size_t width;
    size_t height;
};


//  Check one binary matrix type on a real image:
//  1. `compress_rgb32_to_binary_range()` with one range at a time, and with all of `RANGES`
//     in one pass. Bit (x, y) must be 1 if pixel (x, y) is in range.
//  2. `invert()`, `^=`, `|=`, `&=`, `set_ones()` and `set_zero()` on those matrices.
//  3. `submatrix()` on boxes that start inside or outside a tile, that end at a tile
//     edge or inside a tile, and that go past the edge of the matrix.
//  The image is a guard-page image, so the test also crashes if a compress function reads
//  past the last pixel.
class Test_BinaryMatrix : public UnitTest{
public:
    Test_BinaryMatrix(const std::string& type_name, BinaryMatrixType type, const std::string& image)
        : UnitTest("Kernels::BinaryMatrix - " + type_name + " - " + image)
        , m_type(type)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const ImageRGB32 file_image(m_image);
        const GuardPageImageRGB32 image(file_image);
        const size_t width = image.width();
        const size_t height = image.height();
        auto bit_in_range = [&](size_t range_index){
            return [&, range_index](size_t x, size_t y){
                return in_range(image.pixel(x, y), RANGES[range_index]);
            };
        };

        //  1. Compress, one range at a time.
        std::vector<std::unique_ptr<PackedBinaryMatrix_IB>> matrices;
        for (size_t c = 0; c < std::size(RANGES); c++){
            matrices.emplace_back(make_PackedBinaryMatrix(m_type, width, height));
            compress_rgb32_to_binary_range(
                image.data(), image.bytes_per_row(), *matrices.back(),
                RANGES[c].mins, RANGES[c].maxs
            );
            const std::string error = find_wrong_bit(*matrices.back(), width, height, bit_in_range(c));
            if (!error.empty()){
                return std::format("Error: compress range [{:08x}, {:08x}]: {}.", RANGES[c].mins, RANGES[c].maxs, error);
            }
        }

        //  1. Compress, all ranges in one pass.
        std::vector<std::unique_ptr<PackedBinaryMatrix_IB>> multi_matrices;
        std::vector<CompressRgb32ToBinaryRangeFilter> filters;
        for (const ColorRange& range : RANGES){
            multi_matrices.emplace_back(make_PackedBinaryMatrix(m_type, width, height));
            filters.emplace_back(*multi_matrices.back(), range.mins, range.maxs);
        }
        compress_rgb32_to_binary_range(image.data(), image.bytes_per_row(), filters.data(), filters.size());
        for (size_t c = 0; c < std::size(RANGES); c++){
            const std::string error = find_wrong_bit(*multi_matrices[c], width, height, bit_in_range(c));
            if (!error.empty()){
                return std::format("Error: compress all ranges at once, range [{:08x}, {:08x}]: {}.", RANGES[c].mins, RANGES[c].maxs, error);
            }
        }
        scope.throw_if_cancelled();

        //  2. Operations on the matrices of the first two ranges.
        const PackedBinaryMatrix_IB& a = *matrices[0];
        const PackedBinaryMatrix_IB& b = *matrices[1];
        auto bit_a = bit_in_range(0);
        auto bit_b = bit_in_range(1);

        std::unique_ptr<PackedBinaryMatrix_IB> matrix = a.clone();
        matrix->invert();
        std::string error = find_wrong_bit(*matrix, width, height, [&](size_t x, size_t y){ return !bit_a(x, y); });
        if (!error.empty()){
            return "Error: invert(): " + error + ".";
        }
        matrix = a.clone();
        *matrix ^= b;
        error = find_wrong_bit(*matrix, width, height, [&](size_t x, size_t y){ return bit_a(x, y) != bit_b(x, y); });
        if (!error.empty()){
            return "Error: operator^=(): " + error + ".";
        }
        matrix = a.clone();
        *matrix |= b;
        error = find_wrong_bit(*matrix, width, height, [&](size_t x, size_t y){ return bit_a(x, y) || bit_b(x, y); });
        if (!error.empty()){
            return "Error: operator|=(): " + error + ".";
        }
        matrix = a.clone();
        *matrix &= b;
        error = find_wrong_bit(*matrix, width, height, [&](size_t x, size_t y){ return bit_a(x, y) && bit_b(x, y); });
        if (!error.empty()){
            return "Error: operator&=(): " + error + ".";
        }
        matrix->set_ones();
        error = find_wrong_bit(*matrix, width, height, [](size_t, size_t){ return true; });
        if (!error.empty()){
            return "Error: set_ones(): " + error + ".";
        }
        matrix->set_zero();
        error = find_wrong_bit(*matrix, width, height, [](size_t, size_t){ return false; });
        if (!error.empty()){
            return "Error: set_zero(): " + error + ".";
        }
        scope.throw_if_cancelled();

        //  3. Submatrices of the first range's matrix. The parts past the edge of the matrix
        //  must be 0. The 64 x 64 box ends at a tile edge for every matrix type.
        const SubmatrixBox boxes[] = {
            {0, 0, width, height},
            {1, 1, width - 1, height - 1},
            {5, 3, 64, 64},
            {37, 11, 40, 32},
            {70, 9, 100, 100},
            {width + 3, 0, 10, 10},
        };
        for (const SubmatrixBox& box : boxes){
            const std::unique_ptr<PackedBinaryMatrix_IB> sub = a.submatrix(
                box.x, box.y, box.width, box.height
            );
            error = find_wrong_bit(*sub, box.width, box.height, [&](size_t x, size_t y){
                x += box.x;
                y += box.y;
                return x < width && y < height && bit_a(x, y);
            });
            if (!error.empty()){
                return std::format(
                    "Error: submatrix({}, {}, {}, {}): {}.",
                    box.x, box.y, box.width, box.height, error
                );
            }
        }
        return true;
    }

private:
    BinaryMatrixType m_type;
    std::string m_image;
};

}



//  Add the tile tests, and a matrix test for each image and each binary matrix type that
//  this build has and this CPU supports, including Default.
void add_tests_BinaryMatrix(UnitTestDatabase& database){
    database.add<Test_BinaryTile<BinaryTile_64x4_Default>>("64x4_Default");

    struct MatrixType{
        std::string name;
        BinaryMatrixType type;
    };
    std::vector<MatrixType> types{
        {"64x4_Default", BinaryMatrixType::i64x4_Default},
    };
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        types.push_back({"64x8_x64_SSE42", BinaryMatrixType::i64x8_x64_SSE42});
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        types.push_back({"64x16_x64_AVX2", BinaryMatrixType::i64x16_x64_AVX2});
    }
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        types.push_back({"64x32_x64_AVX512", BinaryMatrixType::i64x32_x64_AVX512});
        types.push_back({"64x64_x64_AVX512", BinaryMatrixType::i64x64_x64_AVX512});
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        database.add<Test_BinaryTile<BinaryTile_64x8_arm64_NEON>>("64x8_arm64_NEON");
        database.add<Test_PartialWordAccess_arm64_NEON>();
        types.push_back({"64x8_arm64_NEON", BinaryMatrixType::arm64x8_x64_NEON});
    }
#endif

    //  Images in the CommandLineTests repo. No width or height is a multiple of 8, so the
    //  tiles at the right and bottom edges are only partly used.
    const std::vector<std::string> images{
        "NintendoSwitch/Switch2/ghostassassin/system_settings_display_dark.png",    //  1916 x 1078
        "OCR/Lv.1-0.png",                                                           //  237 x 77
        "PokemonSwSh/SelectionArrowFinder/mac_neon_error_1.png",                    //  106 x 410
        "OCR/german-nature-sanft.png",                                              //  511 x 91
    };

    for (const MatrixType& type : types){
        for (const std::string& image : images){
            database.add<Test_BinaryMatrix>(type.name, type.type, image);
        }
    }
}



}
}
