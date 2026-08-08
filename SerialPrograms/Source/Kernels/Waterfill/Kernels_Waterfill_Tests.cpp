/*  Waterfill Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#ifdef PA_AutoDispatch_arm64_20_M1
    #include "Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64x8_arm64_NEON.h"
    #include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h"
    #include "Kernels/Waterfill/Kernels_Waterfill_Core_64x8_arm64_NEON.h"
#endif
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64xH_Default.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_64x4_Default.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Core_64xH_Default.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Routines.h"
#include "Kernels_Waterfill_Tests.h"
#include "Tests/TestUtils.h"

#include <iostream>
using std::cout;
using std::endl;


namespace PokemonAutomation{
namespace Kernels{

class Test_Waterfill : public UnitTest{
public:
    Test_Waterfill(
        const std::string& image
    )
        : UnitTest("Kernels::Waterfill - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);

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

        for (size_t i = 0; i < objects.size(); ++i){
            TEST_RESULT_COMPONENT_EQUAL(objects[i].area, gt_objects[i].area, "object " + std::to_string(i) + " area");
            TEST_RESULT_COMPONENT_EQUAL(objects[i].min_x, gt_objects[i].min_x, "object " + std::to_string(i) + " min_x");
            TEST_RESULT_COMPONENT_EQUAL(objects[i].min_y, gt_objects[i].min_y, "object " + std::to_string(i) + " min_y");
            TEST_RESULT_COMPONENT_EQUAL(objects[i].max_x, gt_objects[i].max_x, "object " + std::to_string(i) + " max_x");
            TEST_RESULT_COMPONENT_EQUAL(objects[i].max_y, gt_objects[i].max_y, "object " + std::to_string(i) + " max_y");
        }

        // We try to wait for three seconds:
        const size_t num_iters = size_t(3000 / ms);
        time_start = current_time();
        for (size_t i = 0; i < num_iters; i++){
            matrix = source_matrix.copy();
            objects = Kernels::Waterfill::find_objects_inplace(matrix, min_area);
        }
        time_end = current_time();
        ms = (double)std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
        cout << "Running " << num_iters << " iters, avg filter time: " << ms / num_iters << " ms" << endl;


        return true;
    };

private:
    std::string m_image;
};




void add_tests_Waterfill(UnitTestDatabase& database){



}



}
}
