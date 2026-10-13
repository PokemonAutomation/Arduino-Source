/*  Test Image Distortion Manager
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <atomic>
#include <cmath>
#include <memory>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Filesystem/FilePath.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "TestImageDistortionManager.h"

namespace PokemonAutomation{


namespace{


//  Read a transform written as 3 rows of 4 numbers, one row per output channel (see `ColorTransform3x4`),
//  e.g. the identity [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0]].
//  `label` and `filename` are only used in error messages.
//  Throws ParseException if `json` isn't 3 rows of 4 numbers.
ColorTransform3x4 read_transform(const JsonValue& json, const std::string& label, const std::string& filename){
    const JsonArray& rows = json.to_array_throw(filename);
    if (rows.size() != 3){
        throw ParseException("The transform of \"" + label + "\" in " + filename + " must have 3 rows.");
    }
    ColorTransform3x4 ret;
    for (size_t r = 0; r < 3; r++){
        const JsonArray& row = rows[r].to_array_throw(filename);
        if (row.size() != 4){
            throw ParseException("Each row of the transform of \"" + label + "\" in " + filename + " must have 4 numbers.");
        }
        for (size_t c = 0; c < 4; c++){
            ret[r][c] = row[c].to_double_throw(filename);
        }
    }
    return ret;
}

//  Return `path` with ".", ".." and repeated separators resolved, and '/' as separator, e.g.
//  "PokemonLA/./Map//a.png" -> "PokemonLA/Map/a.png". It's purely textual with no filesystem query.
std::string normalized_path(const std::string& path){
    return Filesystem::Path(path).lexically_normal().string_slash_normalized();
}

//  Return the transform that undoes `transform`: if out = M * in + b, then in = M^-1 * out - M^-1 * b.
//  M^-1 is computed from the adjugate: each entry is a 2x2 cofactor of M divided by det(M).
//  Throws ParseException if M can't be inverted, which a real capture card's transform never is.
ColorTransform3x4 invert(const ColorTransform3x4& transform, const std::string& label){
    const ColorTransform3x4& m = transform;

    //  Cofactors of the first row, which also give the determinant.
    const double c00 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    const double c01 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
    const double c02 = m[1][0] * m[2][1] - m[1][1] * m[2][0];
    const double det = m[0][0] * c00 + m[0][1] * c01 + m[0][2] * c02;
    if (std::abs(det) < 1e-6){
        throw ParseException("The transform of \"" + label + "\" can't be inverted.");
    }

    ColorTransform3x4 ret;
    ret[0][0] = c00 / det;
    ret[1][0] = c01 / det;
    ret[2][0] = c02 / det;
    ret[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) / det;
    ret[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) / det;
    ret[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) / det;
    ret[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) / det;
    ret[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) / det;
    ret[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) / det;
    for (size_t r = 0; r < 3; r++){
        ret[r][3] = -(ret[r][0] * m[0][3] + ret[r][1] * m[1][3] + ret[r][2] * m[2][3]);
    }
    return ret;
}

//  Return the transform that applies `first` and then `second`:
//      second(first(x)) = M2 * (M1 * x + b1) + b2 = (M2 * M1) * x + (M2 * b1 + b2)
//  Colors aren't clipped to 0-255 between the two steps, unlike applying them one at a time.
ColorTransform3x4 compose(const ColorTransform3x4& first, const ColorTransform3x4& second){
    ColorTransform3x4 ret;
    for (size_t r = 0; r < 3; r++){
        for (size_t c = 0; c < 4; c++){
            double sum = c == 3 ? second[r][3] : 0;
            for (size_t k = 0; k < 3; k++){
                sum += second[r][k] * first[k][c];
            }
            ret[r][c] = sum;
        }
    }
    return ret;
}


}



TestImageDistortionManager::TestImageDistortionManager(const std::string& test_repo_path){
    const std::string folder = normalized_path(test_repo_path + "/") + "CaptureCardCalibration/";

    //  Capture setup label -> the transform from the true colors to the colors that setup captures.
    std::map<std::string, ColorTransform3x4> setup_transforms;
    {
        const std::string filename = folder + "color_profile_to_transform.json";
        const JsonValue json = load_json_file(filename);
        for (const auto& item : json.to_object_throw(filename)){
            setup_transforms.emplace(item.first, read_transform(item.second, item.first, filename));
        }
    }

    std::vector<ColorTransform3x4> target_transforms;
    {
        const std::string filename = folder + "color_profile_extremes.json";
        const JsonValue json = load_json_file(filename);
        for (const auto& item : json.to_object_throw(filename)){
            m_test_distortion_labels.emplace_back(item.first);
            target_transforms.emplace_back(read_transform(item.second, item.first, filename));
        }
    }

    {
        const std::string filename = folder + "test_image_labels.json";
        const JsonValue json = load_json_file(filename);
        for (const auto& item : json.to_object_throw(filename)){
            const std::string& label = item.second.to_string_throw(filename);
            if (label == "Unknown"){
                continue;
            }
            if (!setup_transforms.contains(label)){
                throw ParseException(
                    "The test image \"" + item.first + "\" in " + filename + " has the distortion label \"" + label +
                    "\", which has no transform in color_profile_to_transform.json."
                );
            }
            m_image_labels.emplace(item.first, label);
        }
    }

    //  For each capture setup used by a test image, combine undoing its distortion with applying
    //  each test distortion. E.g. for an image captured with "Win-CheapCard" and the test distortion
    //  "Win-MYPIN", the combined transform turns the cheap card's colors into MYPIN's colors.
    for (const auto& item : m_image_labels){
        const std::string& label = item.second;
        if (m_transforms.contains(label)){
            continue;
        }
        const ColorTransform3x4 undo = invert(setup_transforms.at(label), label);
        std::vector<std::optional<ColorTransform3x4>>& per_test_time_transform = m_transforms[label];
        for (size_t c = 0; c < m_test_distortion_labels.size(); c++){
            if (label == m_test_distortion_labels[c]){
                per_test_time_transform.emplace_back(std::nullopt);
            }else{
                per_test_time_transform.emplace_back(compose(undo, target_transforms[c]));
            }
        }
    }
}


size_t TestImageDistortionManager::num_applicable_distortions(const std::vector<std::string>& image_paths) const{
    for (const std::string& path : image_paths){
        if (image_label(path) != nullptr){
            return num_test_distortions();
        }
    }
    return 0;
}


const ColorTransform3x4* TestImageDistortionManager::transform(
    const std::string& image_path,
    const std::string& input_label
) const{
    auto test_labels = std::find(m_test_distortion_labels.begin(), m_test_distortion_labels.end(), input_label);
    if (test_labels == m_test_distortion_labels.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown test distortion label: " + input_label);
    }
    const std::string* label = image_label(image_path);
    if (label == nullptr){
        return nullptr;
    }
    const std::optional<ColorTransform3x4>& ret = m_transforms.at(*label)[test_labels - m_test_distortion_labels.begin()];
    return ret ? &*ret : nullptr;
}


const std::string* TestImageDistortionManager::image_label(const std::string& image_path) const{
    const Filesystem::Path path(image_path);
    if (static_cast<const std::filesystem::path&>(path).is_absolute()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Test image paths must be relative to the test repo folder: " + image_path
        );
    }
    auto iter = m_image_labels.find(normalized_path(image_path));
    return iter == m_image_labels.end() ? nullptr : &iter->second;
}




namespace{

//  The global manager, published by `initialize_test_image_distortions()` and read by the tests,
//  which can run on any thread.
std::atomic<const TestImageDistortionManager*> global_manager(nullptr);

}


void initialize_test_image_distortions(const std::string& test_repo_path){
    static Mutex lock;
    static std::unique_ptr<const TestImageDistortionManager> manager;

    std::lock_guard<Mutex> lg(lock);
    if (manager){
        return;
    }
    manager = std::make_unique<const TestImageDistortionManager>(test_repo_path);
    global_manager.store(manager.get(), std::memory_order_release);
}

const TestImageDistortionManager* test_image_distortion_manager(){
    return global_manager.load(std::memory_order_acquire);
}



}
