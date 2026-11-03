/*  Image Unit Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Base class for unit tests that run on test images, e.g. a detector test that loads a screenshot
 *  and checks the detector's result on it.
 *
 *  It loads the images itself and passes them to `run_on_images()`, which each test implements.
 *  When color distortion augmentation is on ("--augment-color-distortions"), it also runs the test
 *  again on the images re-colored like other capture card / video setups, so one test image covers
 *  more of the color range users see. See Common/Cpp/TestRunners/TestImageDistortionManager.h.
 */

#ifndef PokemonAutomation_Tests_ImageUnitTest_H
#define PokemonAutomation_Tests_ImageUnitTest_H

#include <string>
#include <vector>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"

namespace PokemonAutomation{


//  Upper bound on the in-memory size of a test image as an `ImageRGB32`
//  (width x height x 4): 3840 x 2160 ("4K"), the largest size in the
//  CommandLineTests repository. `ImageUnitTest` declares this many bytes in
//  `UnitTest::m_memory` for the image it loads; tests that keep extra full-size
//  copies of the image, or load more images, add to it in their constructor.
//  A test that loads an image larger than 4K must set `m_memory` to the actual
//  size instead of using this bound.
constexpr uint64_t MAX_TEST_IMAGE_RGB32_BYTES = 3840ull * 2160ull * 4;


//  Best-effort per-test memory estimates for the PaddleOCR (ONNX) pipeline, added to the
//  test image size when setting `UnitTest::m_memory` on tests that run OCR. The ONNX
//  sessions are loaded once per process and shared by all tests, but the estimates
//  attribute the full cost to each test so the parallel test runner's memory budget
//  stays safe. Single-line reads only use the recognition model; multi-line reads
//  additionally load the ~84 MB detection model (PaddleOCRDetection/det.onnx) plus its
//  activations.
constexpr uint64_t PADDLE_OCR_SINGLE_LINE_MEMORY_ESTIMATE = 32ull * 1024 * 1024;
constexpr uint64_t PADDLE_OCR_MULTILINE_MEMORY_ESTIMATE = 256ull * 1024 * 1024;


class ImageUnitTest : public UnitTest{
public:
    //  `name`: the test name, as for `UnitTest`.
    //  `image_paths`: the test images to load, relative to the test repo folder `UNIT_TEST_RESOURCE_PATH()`,
    //  e.g. {"NintendoSwitch/UpdateMenuDetector/English_1_True.png"}.
    ImageUnitTest(std::string name, std::vector<std::string> image_paths);

    //  Load the test images and pass them to `run_on_images()`.
    //  If that passes and color distortion augmentation is on (the global `TestImageDistortionManager`
    //  is loaded), and at least one image has a known capture setup, call `run_on_images()` again once
    //  per test distortion, on the images re-colored with `TestImageDistortionManager::transform()`.
    //  The test passes only if every run passes. Otherwise the message lists each distortion that
    //  failed with the message of that run, e.g. "[Win-MYPIN] Error: ...".
    //  There are no distortion runs while generating golden files (`GENERATE_TEST_GOLDEN_FILES`), so the
    //  golden files hold the results on the images as they are.
    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override final;

    //  The test images, relative to `UNIT_TEST_RESOURCE_PATH()`, as given to the constructor.
    const std::vector<std::string>& image_paths() const{
        return m_image_paths;
    }


protected:
    //  The test itself, implemented by each image test. `images` are the test images in the order
    //  of `image_paths()`: as they are on the first call, then re-colored on the distortion calls.
    //  If an image file is missing, `run()` throws FileException before calling this, which fails the test.
    //  A file that exists but can't be decoded may be passed as an empty image.
    //  It can be called several times in one `run()`, so it must not keep state between calls.
    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const = 0;


private:
    const std::vector<std::string> m_image_paths;
};


}
#endif
