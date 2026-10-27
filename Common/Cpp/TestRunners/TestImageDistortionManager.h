/*  Test Image Distortion Manager
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Each user runs the automation program through a different capture card, OS and Switch video setting.
 *  A visual detector that passes on one test image can fail for another setting.
 *
 *  The CaptureCardCalibration/ folder in the CommandLineTests repo measures these color distortions
 *  (see the README.md there). This manager loads its JSON files:
 *  - color_profile_to_transform.json: distortion label (e.g. "Win-MYPIN") -> 3x4 color transform.
 *  - color_profile_extremes.json: the few "extreme" distortion labels that tests run with.
 *  - test_image_labels.json: test image path (relative to the repo) -> distortion label of the setup
 *    it was captured with, or "Unknown".
 *
 *  For a test image with a known distortion label, the manager provides the color transform to
 *  other extreme distortions. So one test image can cover more of the measured color range.
 *
 *  It is used by SerialPrograms/Source/Tests/ImageUnitTest.cpp.
 */

#ifndef PokemonAutomation_TestImageDistortionManager_H
#define PokemonAutomation_TestImageDistortionManager_H

#include <array>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace PokemonAutomation{



//  An affine color transform on RGB values in 0-255, with one row per output channel:
//      R' = m[0][0] * R + m[0][1] * G + m[0][2] * B + m[0][3]
//      G' = m[1][0] * R + m[1][1] * G + m[1][2] * B + m[1][3]
//      B' = m[2][0] * R + m[2][1] * G + m[2][2] * B + m[2][3]
using ColorTransform3x4 = std::array<std::array<double, 4>, 3>;



class TestImageDistortionManager{
public:
    //  Load the calibration files from "<test_repo_path>/CaptureCardCalibration/".
    //  `test_repo_path` is the CommandLineTests repo folder, i.e. `UNIT_TEST_RESOURCE_PATH()`.
    //  The test image paths given to the other functions are relative to that folder,
    //  e.g. "PokemonLA/StatusInfoScreenDetector/WinMyPin/eng_geodude_NotShiny_NotAlpha_Male.png".
    //  Throws FileException if a file can't be read, and ParseException if a file isn't in the expected
    //  format, a label has no transform, or a test image's transform can't be inverted.
    explicit TestImageDistortionManager(const std::string& test_repo_path);

    //  The number of distortions to apply during testing
    size_t num_test_distortions() const{
        return m_test_distortion_labels.size();
    }

    //  The label name of each test distortion at `index` (`index` < `num_test_distortions()`)
    const std::string& test_distortion_label(size_t index) const{
        return m_test_distortion_labels[index];
    }

    //  How many test distortions can a test based on images from `image_paths` run.
    //  `image_paths` are relative to the test repo folder. Throws InternalProgramError if one is absolute.
    //  It returns `num_test_distortions()` if at least one of the images has a known distortion label, otherwise 0.
    size_t num_applicable_distortions(const std::vector<std::string>& image_paths) const;

    //  The transform that makes the test image at `image_path` (relative to the test repo folder) look as if it was
    //  captured through the setup with label `distortion_label`: it undoes the distortion of the setup the
    //  image was captured with and then applies the distortion of `distortion_label`.
    //  Returns nullptr if the image should be used as it is: it isn't a test image with a known setup,
    //  or it was captured with the `distortion_label` setup itself.
    //  Throws InternalProgramError if `distortion_label` isn't one of the test labels, or if
    //  `image_path` is an absolute path.
    const ColorTransform3x4* transform(const std::string& image_path, const std::string& distortion_label) const;

private:
    //  Return the distortion label of the test image at `image_path` (relative to the test repo
    //  folder), or nullptr if the image has no known label.
    //  Throws InternalProgramError if `image_path` is an absolute path, which would never match.
    const std::string* image_label(const std::string& image_path) const;


private:
    std::vector<std::string> m_test_distortion_labels;

    //  Relative test image path -> distortion label of the setup it was captured with.
    //  Images labeled "Unknown" are left out.
    std::map<std::string, std::string> m_image_labels;

    //  distortion label -> a transform vector of size `m_test_distortion_labels`.
    //  For a key `distortion_label` , the vector of location `i` is the combined transform to transform pixels on an image captured
    //  under distortion, `m_test_distortion_labels[i]` into pixels under distortion `distortion_label`.
    //  The transform is "combined"  because it combines the transform that undistorts the image and the transform that re-distorts it to the
    //  key distortion label.
    //  One transform can be std::nullopt if the combined transform is just an identity transform, aka the source and target distortion
    //  is the same.
    std::map<std::string, std::vector<std::optional<ColorTransform3x4>>> m_transforms;
};



//  Load the global `TestImageDistortionManager` from `test_repo_path` (see its constructor).
//  Call this before running the tests. Once loaded, later calls do nothing.
//  Throws like the constructor of `TestImageDistortionManager`, and stays uninitialized if it does.
void initialize_test_image_distortions(const std::string& test_repo_path);

//  The global `TestImageDistortionManager`, or nullptr if `initialize_test_image_distortions()`
//  hasn't loaded it.
const TestImageDistortionManager* test_image_distortion_manager();



}
#endif
