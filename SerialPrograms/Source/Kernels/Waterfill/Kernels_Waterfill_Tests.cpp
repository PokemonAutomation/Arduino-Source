/*  Waterfill Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Tests/GuardPageImage.h"
#include "Tests/ImageUnitTest.h"
#include "Tests/TestUtils.h"
#include "Kernels_Waterfill_Tests.h"


namespace PokemonAutomation{
namespace Kernels{


namespace Waterfill{
//  The waterfill implementations, defined in Kernels_Waterfill_Core_*.cpp. `find_objects_inplace()`
//  in Kernels_Waterfill.cpp declares the same functions and calls the one that fits the matrix type
//  and the CPU. These tests call them directly, so that every implementation this CPU supports is
//  tested, not only the one that `find_objects_inplace()` picks.
std::vector<WaterfillObject> find_objects_inplace_64x4_Default      (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x8_x64_SSE42    (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x16_x64_AVX2    (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x32_x64_AVX512  (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x64_x64_AVX512  (PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x32_x64_AVX512GF(PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x64_x64_AVX512GF(PackedBinaryMatrix_IB& matrix, size_t min_area);
std::vector<WaterfillObject> find_objects_inplace_64x8_arm64_NEON   (PackedBinaryMatrix_IB& matrix, size_t min_area);
}


//  One waterfill implementation to test: its name in the test name, the binary matrix
//  type it works on, and the function that finds the objects.
struct WaterfillImplementation{
    using FindObjects = std::vector<Waterfill::WaterfillObject> (*)(PackedBinaryMatrix_IB& matrix, size_t min_area);

    std::string name;
    BinaryMatrixType matrix_type;
    FindObjects find_objects_inplace;
};


//  Check one SIMD or NEON waterfill implementation against the Default (no SIMD) one on
//  a real image:
//  1. Turn the image into a binary matrix twice: in the Default 64x4 layout for the
//     reference, and in the layout of the implementation under test. The foreground is
//     the dark pixels, with each of R, G and B in [0, 63].
//  2. Find the objects in each matrix, and sort each list by position. The objects come
//     out in tile order, and the two layouts have different tile sizes.
//  3. Compare the two lists, object by object.
//  The test fails if the reference finds no objects, because then the image tests nothing.
class Test_Waterfill : public ImageUnitTest{
public:
    Test_Waterfill(
        const WaterfillImplementation& implementation,
        const std::string& image
    )
        : ImageUnitTest("Kernels::Waterfill - " + implementation.name + " - " + image, {image})
        , m_implementation(implementation)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        //  Copy the test image into guard-page memory with packed rows, so a
        //  kernel that reads or writes past the last pixel faults.
        const ImageRGB32& file_image = images[0];
        GuardPageImageRGB32 image(file_image);

        std::vector<Waterfill::WaterfillObject> expected = find_sorted_objects(
            image, BinaryMatrixType::i64x4_Default, Waterfill::find_objects_inplace_64x4_Default
        );
        if (expected.empty()){
            return "Error: the reference found no objects, so this image tests nothing.";
        }
        std::vector<Waterfill::WaterfillObject> objects = find_sorted_objects(
            image, m_implementation.matrix_type, m_implementation.find_objects_inplace
        );

        TEST_RESULT_COMPONENT_EQUAL_STR(objects.size(), expected.size(), "number of objects");
        for (size_t i = 0; i < objects.size(); i++){
            const std::string object = "object " + std::to_string(i);
            TEST_RESULT_COMPONENT_EQUAL_STR(objects[i].area, expected[i].area, object + " area");
            TEST_RESULT_COMPONENT_EQUAL_STR(objects[i].min_x, expected[i].min_x, object + " min_x");
            TEST_RESULT_COMPONENT_EQUAL_STR(objects[i].min_y, expected[i].min_y, object + " min_y");
            TEST_RESULT_COMPONENT_EQUAL_STR(objects[i].max_x, expected[i].max_x, object + " max_x");
            TEST_RESULT_COMPONENT_EQUAL_STR(objects[i].max_y, expected[i].max_y, object + " max_y");
            TEST_RESULT_COMPONENT_EQUAL_STR(objects[i].sum_x, expected[i].sum_x, object + " sum_x");
            TEST_RESULT_COMPONENT_EQUAL_STR(objects[i].sum_y, expected[i].sum_y, object + " sum_y");
        }
        return true;
    }

private:
    //  Find the objects in the dark pixels of `image`, with a binary matrix of
    //  `matrix_type` and the waterfill function `find_objects_inplace`. Return them
    //  sorted by position, then by area.
    static std::vector<Waterfill::WaterfillObject> find_sorted_objects(
        const GuardPageImageRGB32& image,
        BinaryMatrixType matrix_type,
        WaterfillImplementation::FindObjects find_objects_inplace
    ){
        std::unique_ptr<PackedBinaryMatrix_IB> matrix = make_PackedBinaryMatrix(
            matrix_type, image.width(), image.height()
        );
        compress_rgb32_to_binary_range(
            image.data(), image.bytes_per_row(), *matrix,
            combine_rgb(0, 0, 0), combine_rgb(63, 63, 63)
        );
        std::vector<Waterfill::WaterfillObject> objects = find_objects_inplace(*matrix, 10);
        std::sort(
            objects.begin(), objects.end(),
            [](const Waterfill::WaterfillObject& a, const Waterfill::WaterfillObject& b){
                return std::tie(a.min_y, a.min_x, a.max_y, a.max_x, a.area)
                     < std::tie(b.min_y, b.min_x, b.max_y, b.max_x, b.area);
            }
        );
        return objects;
    }

private:
    WaterfillImplementation m_implementation;
};




//  Add a test for each image and each SIMD or NEON waterfill implementation that this
//  build has and this CPU supports. The Default implementation is the reference, so it
//  gets no test of its own.
void add_tests_Waterfill(UnitTestDatabase& database){
    using namespace Waterfill;

    std::vector<WaterfillImplementation> implementations;
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        implementations.push_back({"64x8_x64_SSE42", BinaryMatrixType::i64x8_x64_SSE42, find_objects_inplace_64x8_x64_SSE42});
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        implementations.push_back({"64x16_x64_AVX2", BinaryMatrixType::i64x16_x64_AVX2, find_objects_inplace_64x16_x64_AVX2});
    }
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        implementations.push_back({"64x32_x64_AVX512", BinaryMatrixType::i64x32_x64_AVX512, find_objects_inplace_64x32_x64_AVX512});
        implementations.push_back({"64x64_x64_AVX512", BinaryMatrixType::i64x64_x64_AVX512, find_objects_inplace_64x64_x64_AVX512});
    }
#endif
#ifdef PA_AutoDispatch_x64_19_IceLake
    if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
        implementations.push_back({"64x32_x64_AVX512GF", BinaryMatrixType::i64x32_x64_AVX512, find_objects_inplace_64x32_x64_AVX512GF});
        implementations.push_back({"64x64_x64_AVX512GF", BinaryMatrixType::i64x64_x64_AVX512, find_objects_inplace_64x64_x64_AVX512GF});
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        implementations.push_back({"64x8_arm64_NEON", BinaryMatrixType::arm64x8_x64_NEON, find_objects_inplace_64x8_arm64_NEON});
    }
#endif

    //  Images in the CommandLineTests repo. None has a width or height that is a multiple
    //  of 64, so the tiles at the right and bottom edges are only partly used.
    const std::vector<std::string> images{
        "NintendoSwitch/Switch2/ghostassassin/system_settings_display_dark.png",    //  1916 x 1078
        "OCR/sentence-1-1-tall.jpg",                                                //  564 x 500
        "PokemonSwSh/SelectionArrowFinder/mac_neon_error_1.png",                    //  106 x 410
    };

    for (const WaterfillImplementation& implementation : implementations){
        for (const std::string& image : images){
            database.add<Test_Waterfill>(implementation, image);
        }
    }
}



}
}
