/*  Test Image Distortion
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Re-color test images as if they were captured through a different capture card / video setup, so that
 *  detector tests also cover the color distortions of other users' settings.
 *  See Common/Cpp/TestRunners/TestImageDistortionManager.h for how this works.
 */

#ifndef PokemonAutomation_Tests_TestImageDistortion_H
#define PokemonAutomation_Tests_TestImageDistortion_H

#include <string>

namespace PokemonAutomation{

class ImageRGB32;


//  Re-color `image`, loaded from the test image at `image_path`, as if it was captured through the
//  capture card / video setup of the test distortion `distortion_label`.
//  `image_path`: relative to the test repo folder `UNIT_TEST_RESOURCE_PATH()`, e.g.
//  "NintendoSwitch/UpdateMenuDetector/English_1_True.png".
//  The actual transform comes from the global `TestImageDistortionManager`, see 
//  Common/Cpp/TestRunners/TestImageDistortionManager.h for more details.
//
//  Each image unit test needs to call this after loading each test image to augment the test.
//
//  The function does nothing when `distortion_label` is empty (which means this is a test run without
//  distortion), or when the input image is found to already be captured via the distortion label refers to. 
//  Throws InternalProgramError if `distortion_label` is set but the manager isn't initialized, or
//  isn't one of the manager's test distortion labels, or if `image_path` is absolute.
void apply_test_image_distortion(
    ImageRGB32& image,
    const std::string& image_path,
    const std::string& distortion_label
);


}
#endif
