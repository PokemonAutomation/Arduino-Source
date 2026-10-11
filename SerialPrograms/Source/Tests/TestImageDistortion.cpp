/*  Test Image Distortion
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <opencv2/core.hpp>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/TestRunners/TestImageDistortionManager.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32_OpenCV.h"
#include "TestImageDistortion.h"

namespace PokemonAutomation{


void apply_test_image_distortion(
    ImageRGB32& image,
    const std::string& image_path,
    const std::string& distortion_label
){
    if (distortion_label.empty() || !image){
        return;
    }
    const TestImageDistortionManager* manager = test_image_distortion_manager();
    if (manager == nullptr){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Test image distortion \"" + distortion_label + "\" requested, but the distortions are not initialized."
        );
    }
    const ColorTransform3x4* transform = manager->transform(image_path, distortion_label);
    if (transform == nullptr){
        // the image is already captured under the same distortion setting the label refers to
        return;
    }

    //  `ImageRGB32` stores each pixel as a uint32_t 0xAARRGGBB, so its bytes in memory (little
    //  endian) are B, G, R, A: OpenCV's BGRA. `cv::transform()` multiplies each pixel's 4 channels
    //  and a constant 1 by a 4x5 matrix. Reorder the RGB transform's rows (outputs) and columns
    //  (inputs) to BGR, and add a row that keeps alpha as it is:
    //      B' = m[2][2]*B + m[2][1]*G + m[2][0]*R + 0*A + m[2][3]
    //      G' = m[1][2]*B + m[1][1]*G + m[1][0]*R + 0*A + m[1][3]
    //      R' = m[0][2]*B + m[0][1]*G + m[0][0]*R + 0*A + m[0][3]
    //      A' =                                     1*A
    const ColorTransform3x4& m = *transform;
    cv::Matx<float, 4, 5> bgra_matrix = cv::Matx<float, 4, 5>::zeros();
    for (int out = 0; out < 3; out++){
        const int rgb_row = 2 - out;
        for (int in = 0; in < 3; in++){
            bgra_matrix(out, in) = (float)m[rgb_row][2 - in];
        }
        bgra_matrix(out, 4) = (float)m[rgb_row][3];
    }
    bgra_matrix(3, 3) = 1;

    //  `cv::transform()` rounds the results and clips them to 0-255.
    ImageRGB32 distorted(image.width(), image.height());
    cv::Mat output = to_OpenCV_ref(distorted);
    // cv::transform handles clipping and rounding
    cv::transform(to_OpenCV_ref(image), output, bgra_matrix);
    image = std::move(distorted);
}


}
