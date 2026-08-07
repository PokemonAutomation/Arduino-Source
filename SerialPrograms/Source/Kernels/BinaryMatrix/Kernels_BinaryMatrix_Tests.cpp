/*  BinaryMatrix Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/GlobalAutoPaths.h"
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
#include "Kernels_BinaryMatrix_Tests.h"

#include <functional>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::flush;

namespace PokemonAutomation{
namespace Kernels{

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
    for (size_t height = 0; height < tile.HEIGHT; ++height){
        for (size_t width = 0; width < tile.WIDTH; ++width){
            tile.set_zero();
            tile.set_ones(width, height);
            for (size_t y = 0; y < tile.HEIGHT; ++y){
                for (size_t x = 0; x < tile.WIDTH; ++x){
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
    for (size_t i = 0; i < num_iters; ++i){
        tile.set_ones(32, 4);
        sum += tile.row(0) + tile.row(1) + tile.row(2) + tile.row(3);
    }
    time_end = current_time();
    ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
    ms = ns / 1000000.;
    cout << "Execution enforcer: " << sum << endl;
    cout << "Running " << num_iters << " iters, Tile::set_ones() time: " << ms / num_iters << " ms" << endl;

    for (size_t height = 0; height < tile.HEIGHT; ++height){
        for (size_t width = 0; width < tile.WIDTH; ++width){
            tile.set_ones();
            tile.clear_padding(width, height);
            for (size_t y = 0; y < tile.HEIGHT; ++y){
                for (size_t x = 0; x < tile.WIDTH; ++x){
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
    for (size_t i = 0; i < num_iters; ++i){
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
    for (size_t y = 0; y < tile.HEIGHT; ++y){
        for (size_t x = 0; x < tile.WIDTH; ++x){
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
    for (size_t y = 0; y < tile.HEIGHT; ++y){
        for (size_t x = 0; x < tile.WIDTH; ++x){
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
        for (size_t x = 0; x < tile.WIDTH; ++x){
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
        for (size_t x = 0; x < tile.WIDTH; ++x){
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
        for (size_t x = 0; x < tile.WIDTH; ++x){
            uint64_t bit = ((bottom >> x) & 1);
            if (bit != 1){
                cout << "& Tile::bottom(), tile (x = " <<
                    x << "), wrong bit. Should be 1" << endl;
                return 1;
            }
        }
    }
    tile.set_zero();
    for (size_t y = 0; y < tile.HEIGHT; ++y){
        for (size_t x = 0; x < tile.WIDTH; ++x){
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
    for (size_t num_tests = 0; num_tests < 30; ++num_tests){
        src_tile.set_zero();
        dst_tile.set_zero();
        for (size_t y = 0; y < tile.HEIGHT; ++y){
            for (size_t x = 0; x < tile.WIDTH; ++x){
                src_tile.set_bit(x, y, std::rand() % 2);
                dst_tile.set_bit(x, y, std::rand() % 2);
            }
        }

        auto test_copy_to_shift = [&](size_t shift_x, size_t shift_y, const std::string& function_name,
            std::function<bool(size_t, size_t, size_t, size_t)> gt_test,
            std::function<bool(size_t, size_t, size_t, size_t)> get_src_bit_test){
                // check against GT
                for (size_t y = 0; y < tile.HEIGHT; ++y){
                    for (size_t x = 0; x < tile.WIDTH; ++x){
                        bool gt = false;
                        if (gt_test(shift_x, shift_y, x, y)){
                            gt = dst_tile.get_bit(x, y) || get_src_bit_test(shift_x, shift_y, x, y);
                            // cout << "set the source bit with ||" << endl;
                        } else{
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

        for (size_t shift_y = 0; shift_y < tile.HEIGHT; ++shift_y){
            for (size_t shift_x = 0; shift_x < tile.WIDTH; ++shift_x){
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

    for (size_t num_bytes = 0; num_bytes <= 16; ++num_bytes){
        PartialWordAccess_arm64_NEON partial(num_bytes);

        uint8_t buffer[48];
        for (int i = 0; i < 48; i++){
            buffer[i] = 99;
        }
        for (uint8_t i = 0; i < 16; i++){
            buffer[i + 16] = i;
        }

        // uint8x16_t x = partial.load(buffer+16);
        uint8x16_t x = partial.load_int_no_read_past_end(buffer + 16);
        for (size_t i = 0; i < num_bytes; ++i){
            if (x[i] != buffer[i + 16]){
                cout << "Error: PartialWordAccess_arm64_NEON(" << num_bytes << ")::load_int_no_read_past_end(), i = " << i << " is " << int(x[i])
                    << ", but should be " << int(buffer[i + 16]) << endl;
                return 1;
            }
        }
        x = partial.load_int_no_read_before_ptr(buffer + 16);
        for (size_t i = 0; i < num_bytes; ++i){
            if (x[i] != buffer[i + 16]){
                cout << "Error: PartialWordAccess_arm64_NEON(" << num_bytes << ")::load_int_no_read_before_ptr(), i = " << i << " is " << int(x[i])
                    << ", but should be " << int(buffer[i + 16]) << endl;
                return 1;
            }
        }
        for (int i = 0; i < 48; i++){
            buffer[i] = 99;
        }
        partial.store_int_no_past_end(buffer + 16, x);
        for (size_t i = 0; i < num_bytes; ++i){
            if (x[i] != buffer[i + 16]){
                cout << "Error: PartialWordAccess_arm64_NEON(" << num_bytes << ")::store_int_no_past_end(), i = " << i << " is " << int(buffer[i + 16])
                    << ", but should be " << int(x[i + 16]) << endl;
                return 1;
            }
        }
    }
#endif
    return 0;
}


class Test_BinaryMatrix : public UnitTest{
public:
    Test_BinaryMatrix(
        const std::string& image
    )
        : UnitTest("Kernels::BinaryMatrix - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);

        if (test_binary_matrix_tile() != 0){
            return false;
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
            return false;
        }


        // We try to wait for three seconds:
        const size_t num_iters = size_t(3000 / ms);
        time_start = current_time();
        for (size_t i = 0; i < num_iters; i++){
            compress_rgb32_to_binary_range(
                image.data(), image.bytes_per_row(), *binary_matrix, mins, maxs
            );
        }
        time_end = current_time();
        ms = (double)std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
        cout << "Running " << num_iters << " iters, average creation impl. time: " << ms / (double)num_iters << " ms" << endl;

        // cout << binary_matrix->dump() << flush;

        return true;
    };

private:
    std::string m_image;
};

void add_tests_BinaryMatrix(UnitTestDatabase& database){

}



}
}
