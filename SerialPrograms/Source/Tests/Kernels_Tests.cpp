/*  Kernels Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Compiler.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "Kernels/ImageScaleBrightness/Kernels_ImageScaleBrightness.h"
#include "Kernels_Tests.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{

using namespace Kernels;

// using namespace NintendoSwitch::PokemonLA;
int test_kernels_ImageScaleBrightness(const ImageViewRGB32& image){
    ImageRGB32 new_image = image.copy();
    
    int num_iterations = 5000;
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

}
