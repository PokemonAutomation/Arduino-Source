/*  Image Unit Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <opencv2/core.hpp>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Filesystem/FilePath.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "Common/Cpp/TestRunners/TestImageDistortionManager.h"
#include "CommonFramework/ImageTypes/ImageRGB32_OpenCV.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/StaticGlobals.h"
#include "ImageUnitTest.h"

namespace PokemonAutomation{


namespace{

//  Return a copy of `image` with `transform` applied to the RGB of every pixel. Alpha is kept as it is.
//  `ImageUnitTest::run()` uses this to re-color a test image as if it was captured through another
//  capture card / video setup, with a transform from `TestImageDistortionManager::transform()`.
//  The whole image is done in one `cv::transform()` call.
ImageRGB32 apply_color_transform(const ImageViewRGB32& image, const ColorTransform3x4& transform){
    if (!image){
        return ImageRGB32();
    }

    //  `ImageRGB32` stores each pixel as a uint32_t 0xAARRGGBB, so its bytes in memory (little
    //  endian) are B, G, R, A: OpenCV's BGRA. `cv::transform()` multiplies each pixel's 4 channels
    //  and a constant 1 by a 4x5 matrix. Reorder the RGB transform's rows (outputs) and columns
    //  (inputs) to BGR, and add a row that keeps alpha as it is:
    //      B' = m[2][2]*B + m[2][1]*G + m[2][0]*R + 0*A + m[2][3]
    //      G' = m[1][2]*B + m[1][1]*G + m[1][0]*R + 0*A + m[1][3]
    //      R' = m[0][2]*B + m[0][1]*G + m[0][0]*R + 0*A + m[0][3]
    //      A' =                                     1*A
    const ColorTransform3x4& m = transform;
    cv::Matx<float, 4, 5> bgra_matrix = cv::Matx<float, 4, 5>::zeros();
    for (int out = 0; out < 3; out++){
        const int rgb_row = 2 - out;
        for (int in = 0; in < 3; in++){
            bgra_matrix(out, in) = (float)m[rgb_row][2 - in];
        }
        bgra_matrix(out, 4) = (float)m[rgb_row][3];
    }
    bgra_matrix(3, 3) = 1;

    ImageRGB32 distorted(image.width(), image.height());
    cv::Mat output = to_OpenCV_ref(distorted);
    // cv::transform handles clipping and rounding
    cv::transform(to_OpenCV_ref(image), output, bgra_matrix);
    return distorted;
}

//  Parse the test category from a test name. e.g.:
//  "PokemonLA::MMOSpriteMatcher - PokemonLA/MMOSpriteMatcher/macOS_bright/Highlands/MMO_Region_Highlands_03.png"
//  -> "PokemonLA-MMOSpriteMatcher".
//  This is useful for debugging.
std::string parse_test_category_name(const std::string& test_name){
    std::string category = test_name.substr(0, test_name.find(" - "));
    for (size_t pos = category.find("::"); pos != std::string::npos; pos = category.find("::", pos + 1)){
        category.replace(pos, 2, "-");
    }
    return category;
}

}


ImageUnitTest::ImageUnitTest(std::string name, std::vector<std::string> image_paths)
    : UnitTest(std::move(name))
    , m_image_paths(std::move(image_paths))
    , m_test_category_name(parse_test_category_name(UnitTest::name()))
{
    //  The upper bound on the in-memory size of the test images this test loads.
    //  Tests that keep extra full-size copies of the images, or load more images,
    //  add to this in their constructor.
    m_memory = MAX_TEST_IMAGE_RGB32_BYTES * m_image_paths.size();
}


UnitTestResult ImageUnitTest::run(Logger& logger, CancellableScope& scope) const{
    std::vector<ImageRGB32> images;
    std::vector<std::string> image_filenames; // used for debugging
    for (const std::string& path : m_image_paths){
        images.emplace_back(UNIT_TEST_RESOURCE_PATH() + path);
        image_filenames.emplace_back(Filesystem::Path(path).filename().string());
    }

    //  Run on the images as they are. Only a test that passes is worth running with distortions.
    UnitTestResult result = run_on_images(logger, scope, images);
    const TestImageDistortionManager* manager = test_image_distortion_manager();
    if (result.result != UnitTestResult::PASSED || manager == nullptr){
        return result;
    }

    //  When generating golden files, a test writes its results on the images as the expected
    //  results. We don't need to run image augmented tests.
    if (STATIC_GLOBALS.GENERATE_TEST_GOLDEN_FILES){
        return result;
    }

    //  Run again on the images with distortion augmentation applied:
    std::string failures;
    std::vector<std::string> failed_distortion_labels;
    const size_t distortions = manager->num_applicable_distortions(m_image_paths);
    for (size_t c = 0; c < distortions; c++){
        scope.throw_if_cancelled();
        const std::string& distortion_label = manager->test_distortion_label(c);
        const std::string run_name = name() + " [" + distortion_label + "]";

        std::vector<ImageRGB32> distorted_images;
        for (size_t i = 0; i < images.size(); i++){
            const ColorTransform3x4* transform = manager->transform(m_image_paths[i], distortion_label);
            if (transform == nullptr){
                //  The image has no known capture setup, or is already captured under the same
                //  distortion setting the distortion_label refers to.
                distorted_images.emplace_back(images[i].copy());
            }else{
                distorted_images.emplace_back(apply_color_transform(images[i], *transform));
            }
        }

        //  Turn an exception into a failure of this distortion, so the message says which one threw.
        UnitTestResult distorted_result;
        try{
            distorted_result = run_on_images(logger, scope, distorted_images);
        }catch (Exception& e){
            distorted_result = UnitTestResult(UnitTestResult::FAILED, e.to_str());
        }

        if (distorted_result.result == UnitTestResult::PASSED){
            logger.log(run_name + ": passed", COLOR_BLUE);
            continue;
        }
        // The test failed with distortion augmentation:
        logger.log(run_name + ": failed, Message: " + distorted_result.message, COLOR_RED);
        failures += "\n- [" + distortion_label + "] " + distorted_result.message;
        failed_distortion_labels.push_back(distortion_label);
        //  Save the augmented images to help debug the failure, e.g.
        //  DEBUG_PATH()/UnitTest/PokemonLA-MMOSpriteMatcher/Win-MYPIN-MMO_Region_Highlands_03.png
        const std::string debug_folder_path = UNIT_TEST_DEBUG_PATH() + m_test_category_name;
        Filesystem::create_directories(debug_folder_path);
        for (size_t i = 0; i < distorted_images.size(); i++){
            const std::string full_path = debug_folder_path + "/[" + distortion_label + "]-" + image_filenames[i];
            logger.log("Save augmented image: " + full_path, COLOR_YELLOW);
            distorted_images[i].save(full_path);
        }
    }

    if (failed_distortion_labels.empty()){
        return result;
    }
    UnitTestResult failed(UnitTestResult::FAILED, "Failed on following image distortions:" + failures);
    return failed;
}


}
