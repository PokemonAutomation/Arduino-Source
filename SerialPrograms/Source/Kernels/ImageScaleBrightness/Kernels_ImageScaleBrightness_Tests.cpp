/*  Image Scale Brightness Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/Color.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
//#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64xH_Default.h"
#include "Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness.h"
#include "Tests/TestImageDistortion.h"
#include "Kernels_ImageScaleBrightness_Tests.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{

class Test_ImageScaleBrightness : public UnitTest{
public:
    Test_ImageScaleBrightness(
        const std::string& image
    )
        : UnitTest("Kernels::ImageScaleBrightness - " + image)
        , m_image(image)
    {}

    virtual std::vector<std::string> image_paths() const override{
        return {m_image};
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 new_image(UNIT_TEST_RESOURCE_PATH() + m_image);
        apply_test_image_distortion(new_image, m_image, distortion_label());

        int num_iterations = 500;
        auto time_start = current_time();
        for (int i = 0; i < num_iterations; i++){
            scale_brightness(new_image.width(), new_image.height(), new_image.data(), new_image.bytes_per_row(), 1.2f, 1.3f, 0.5f);
            // break;
        }
        auto time_end = current_time();
        const auto ms = std::chrono::duration_cast<Milliseconds>(time_end - time_start).count();
        cout << "Time: " << ms << " ms, " << ms / 1000. << " s" << endl;

        // new_image.save("./output.png");

        return true;
    };

private:
    std::string m_image;
};

void add_tests_ImageScaleBrightness(UnitTestDatabase& database){

}



}
}
