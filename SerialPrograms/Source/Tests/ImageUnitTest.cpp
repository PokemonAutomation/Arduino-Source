/*  Image Unit Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <opencv2/core.hpp>
#include <cstring>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Filesystem/FileIO.h"
#include "Common/Cpp/TestRunners/TestImageDistortionManager.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/ImageTypes/ImageRGB32_OpenCV.h"
#include "ImageUnitTest.h"

namespace PokemonAutomation{


namespace{

//  Big-endian 16/32-bit reads from a byte buffer.
uint16_t be16(const uint8_t* p){
    return (uint16_t)((uint16_t)p[0] << 8 | p[1]);
}
uint32_t be32(const uint8_t* p){
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3];
}

//  PNG: the 24-byte signature + IHDR header holds the dimensions at offsets 16 and 20.
bool png_dimensions(const std::string& data, size_t& width, size_t& height){
    static const uint8_t SIGNATURE[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    if (data.size() < 24 || std::memcmp(data.data(), SIGNATURE, 8) != 0){
        return false;
    }
    width = be32(reinterpret_cast<const uint8_t*>(data.data()) + 16);
    height = be32(reinterpret_cast<const uint8_t*>(data.data()) + 20);
    return width != 0 && height != 0;
}

//  JPEG: scan the marker stream for a SOF0..SOF15 frame (0xC0..0xCF, excluding the
//  DHT / JPG / DAC markers 0xC4, 0xC8 and 0xCC); its payload is height then width.
bool jpeg_dimensions(const std::string& data, size_t& width, size_t& height){
    if (data.size() < 4 || (uint8_t)data[0] != 0xff || (uint8_t)data[1] != 0xd8){
        return false;
    }
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data.data());
    size_t i = 2;
    while (i + 4 < data.size()){
        if (bytes[i] != 0xff){
            i++;
            continue;
        }
        const uint8_t marker = bytes[i + 1];
        if (marker == 0xff || (marker >= 0xd0 && marker <= 0xd7) || marker == 0x01 || marker == 0xd9){
            i += 2;
            continue;
        }
        const size_t length = be16(bytes + i + 2);
        if ((marker >= 0xc0 && marker <= 0xcf) && marker != 0xc4 && marker != 0xc8 && marker != 0xcc){
            if (i + 9 > data.size()){
                return false;
            }
            height = be16(bytes + i + 5);
            width = be16(bytes + i + 7);
            return width != 0 && height != 0;
        }
        i += 2 + length;
    }
    return false;
}

}


uint64_t test_image_rgb32_bytes(const std::string& relative_path){
    std::string data;
    if (!file_to_string(UNIT_TEST_RESOURCE_PATH() + relative_path, data)){
        return 0;
    }
    if (data.size() < 24){
        return 0;
    }
    size_t width = 0, height = 0;
    if (png_dimensions(data, width, height)){
        return (uint64_t)width * (uint64_t)height * 4;
    }
    if (jpeg_dimensions(data, width, height)){
        return (uint64_t)width * (uint64_t)height * 4;
    }
    return 0;
}


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

}


ImageUnitTest::ImageUnitTest(std::string name, std::vector<std::string> image_paths)
    : UnitTest(std::move(name))
    , m_image_paths(std::move(image_paths))
{}


UnitTestResult ImageUnitTest::run(Logger& logger, CancellableScope& scope) const{
    std::vector<ImageRGB32> images;
    for (const std::string& path : m_image_paths){
        images.emplace_back(UNIT_TEST_RESOURCE_PATH() + path);
    }

    //  Run on the images as they are. Only a test that passes is worth running with distortions.
    UnitTestResult result = run_on_images(logger, scope, images);
    const TestImageDistortionManager* manager = test_image_distortion_manager();
    if (result.result != UnitTestResult::PASSED || manager == nullptr){
        return result;
    }

    //  When generating golden files, a test writes its results on the images as the expected
    //  results. The distortion runs would overwrite them with results on re-colored images.
    if (STATIC_GLOBALS.GENERATE_TEST_GOLDEN_FILES){
        return result;
    }

    //  Run again on the images re-colored like each test distortion, and collect the failures.
    std::string failures;
    const size_t distortions = manager->num_applicable_distortions(m_image_paths);
    for (size_t c = 0; c < distortions; c++){
        scope.throw_if_cancelled();
        const std::string& label = manager->test_distortion_label(c);
        const std::string run_name = name() + " [" + label + "]";

        std::vector<ImageRGB32> distorted_images;
        for (size_t i = 0; i < images.size(); i++){
            const ColorTransform3x4* transform = manager->transform(m_image_paths[i], label);
            if (transform == nullptr){
                //  The image has no known capture setup, or is already captured under the same
                //  distortion setting the label refers to.
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
        logger.log(run_name + ": failed, Message: " + distorted_result.message, COLOR_RED);
        failures += "\n[" + label + "] " + distorted_result.message;
    }

    if (failures.empty()){
        return result;
    }
    return UnitTestResult(UnitTestResult::FAILED, "Failed with color distortions:" + failures);
}


}
