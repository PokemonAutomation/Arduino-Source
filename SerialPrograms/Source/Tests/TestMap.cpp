/*  Test Map
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "CommonFramework_Tests.h"
#include "Kernels_Tests.h"
#include "NintendoSwitch_Tests.h"
#include "PokemonLA_Tests.h"
#include "PokemonLZA_Tests.h"
#include "PokemonSwSh_Tests.h"
#include "PokemonSV_Tests.h"
#include "TestMap.h"
#include "TestUtils.h"
#include "CommonFramework/AudioPipeline/AudioTemplate.h"

#include <QFileInfo>

#include <iostream>
#include <algorithm>
#include <string>
using std::cout;
using std::cerr;
using std::endl;

using namespace std::placeholders;  // for _1, _2, _3... with std::bind()

namespace PokemonAutomation{


// For each test object, TestFunction will do the work of skipping non-test
// files, read data from test file names or gold files and possibly more.
// To reuse the code, we design the following helper functions.

using ImageFilenameFunction = std::function<int(const ImageViewRGB32& image, const std::string& filename_base)>;

using ImageBoolDetectorFunction = std::function<int(const ImageViewRGB32& image, bool target)>;

using ImageFloatDetectorFunction = std::function<int(const ImageViewRGB32& image, float target, float threshold)>;

using ImageIntDetectorFunction = std::function<int(const ImageViewRGB32& image, int target)>;

using ImageWordsDetectorFunction = std::function<int(const ImageViewRGB32& image, const std::vector<std::string>& words)>;

using ImageVoidDetectorFunction = std::function<int(const ImageViewRGB32& image)>;

using SoundBoolDetectorFunction = std::function<int(const std::vector<AudioSpectrum>& spectrums, bool target)>;

// Basic check on whether an image can be loaded.
// Also strip the image format suffix (.png and so on)

// Helper for testing code that reads an image and uses filename to get the target outcome for the code.
// test_func: reads an image and the image filename base and returns an int code.
int image_filename_detector_helper(ImageFilenameFunction test_func, const std::string& test_path){
    ImageRGB32 image;
    std::string basename;
    try{
        image = ImageRGB32(test_path);
        
        QFileInfo file_info(QString::fromStdString(test_path));
        std::string full_name = file_info.fileName().toStdString();

        // Find the basename (full name without file extension)
        // QFileInfo has a function basename() to get basename, but we shouldn't use it
        // because for a filename like Free_0.421_0.024_0.163_0.174.png where floating point values are
        // put into the filename, QFileInfo::basename() will return a basename of "Free_0". It finds the
        // first "." as the separation for extension, not the last.
        size_t dot_loc = full_name.find_last_of(".");
        basename = full_name.substr(0, dot_loc);
        cout << "Parse file path: file name: " << full_name << ", base name: " << basename << endl;
    }catch (FileException&){
        cout << "Skip " << test_path << " as it cannot be read as image" << endl;
        return -1;
    }
    return test_func(image, basename);
}


// Helper for testing detector code that reads an image and returns true or false.
// The target test result (whether this test file should be detected as true or false)
// is stored as part of the filename. For example, IngoBattleDayTime_True.png.
int image_bool_detector_helper(ImageBoolDetectorFunction test_func, const std::string& test_path){
    auto parse_filename_and_run_test = [&](const ImageViewRGB32& image, const std::string& filename_base){
        const auto name_base = QString::fromStdString(filename_base);
        bool target_bool = false;
        if (name_base.endsWith("_True")){
            target_bool = true;
        }else if (name_base.endsWith("_False")){
            target_bool = false;
        }else{
            cerr << "Error: image test file " << test_path << " has incorrect target detection result (_True/_False) set in the filename." << endl;
            return 1;
        }

        return test_func(image, target_bool);
    };

    return image_filename_detector_helper(parse_filename_and_run_test, test_path);
}

// Helper for testing detector code that reads an image and returns some custom data that can be described
// by words included in the test filename.
// The helper will split the filename by "_" into words and send it in the same order to the test function.
int image_words_detector_helper(ImageWordsDetectorFunction test_func, const std::string& test_path){
    auto parse_filename_and_run_test = [&](const ImageViewRGB32& image, const std::string& filename_base){
        return test_func(image, parse_words(filename_base));
    };

    return image_filename_detector_helper(parse_filename_and_run_test, test_path);
}

// Helper for testing detector code that reads an image and returns a non-negative float that can be described
// in the filename for example <name_base>_0.4.png.
int image_non_negative_float_detector_helper(ImageFloatDetectorFunction test_func, const std::string& test_path){
    auto parse_filename_and_run_test = [&](const ImageViewRGB32& image, const std::vector<std::string>& words) -> int{
        if (words.size() < 2){
            cerr << "Error: image test file " << test_path << " does not have two non-negative floats (e.g image_0.4_0.001.png) set in the filename." << endl;
            return 1;
        }

        float target_number = 0.0f, threshold = 0.0f;

        if (parse_float(words[words.size()-2], target_number) == false || parse_float(words[words.size()-1], threshold) == false){
            cerr << "Error: image test file " << test_path << " does not have two non-negative floats (e.g image_0.4_0.001.png) set in the filename." << endl;
            return 1;
        }

        return test_func(image, target_number, threshold);
    };

    return image_words_detector_helper(parse_filename_and_run_test, test_path);
}

int image_int_detector_helper(ImageIntDetectorFunction test_func, const std::string& test_path){
    auto parse_filename_and_run_test = [&](const ImageViewRGB32& image, const std::vector<std::string>& words) -> int{
        if (words.size() == 0){
            cerr << "Error: image test file " << test_path << " does not have an int (e.g image_5.png) set in the filename." << endl;
            return 1;
        }

        int target_number = 0;

        if (parse_int(words[words.size()-1], target_number) == false){
            cerr << "Error: image test file " << test_path << " does not have an int (e.g image_5.png) set in the filename." << endl;
            return 1;
        }

        return test_func(image, target_number);
    };

    return image_words_detector_helper(parse_filename_and_run_test, test_path);
}


// Helper for testing sdetector code that reads an image and returns nothing.
// This is used for developing visual inference code where the developer writes custom
// debugging output. So no need to get target values from the test framework.
int image_void_detector_helper(ImageVoidDetectorFunction test_func, const std::string& test_path){
    auto run_test = [&](const ImageViewRGB32& image, const std::string&) -> int{
        return test_func(image);
    };

    return image_filename_detector_helper(run_test, test_path);
}


// Basic check on whether an image can be loaded.
// Also strip the image format suffix (.png and so on)

int sound_bool_detector_helper(SoundBoolDetectorFunction test_func, const std::string& test_path){
    QFileInfo file_info(QString::fromStdString(test_path));
    std::string filename = file_info.fileName().toStdString();

    // Search for the target test result from test filename.
    const size_t target_pos = filename.rfind('.');
    if (target_pos == std::string::npos){
        cerr << "Error: image test file " << test_path << " has no \".\" in the filename." << endl;
        return 1;
    }

    // cout << "Test file: " << test_path << endl;
    const auto filename_base = filename.substr(0, target_pos);

    const auto name_base = QString::fromStdString(filename_base);
    bool target_bool = false;
    if (name_base.endsWith("_True")){
        target_bool = true;
    }else if (name_base.endsWith("_False")){
        target_bool = false;
    }else{
        cerr << "Error: audio test file " << test_path << " has incorrect target detection result (_True/_False) set in the filename." << endl;
        return 1;
    }



    // XXX for now we assume the audio in the command line test is always 48000.
    //     in future we can read sample rate from filename
    size_t sample_rate = 48000;
    AudioTemplate audio_stream = loadAudioTemplate(test_path, sample_rate);
    std::vector<AudioSpectrum> spectrums;
    for(size_t i = 0; i < audio_stream.numWindows(); i++){
        // AudioSpectrum(size_t s, size_t rate, std::shared_ptr<const AlignedVector<float>> m);
        AlignedVector<float> freq_mag(audio_stream.numFrequencies());
        memcpy(freq_mag.data(), audio_stream.getWindow(i), sizeof(float) * audio_stream.numFrequencies());
        spectrums.emplace_back(0, sample_rate, std::make_shared<const AlignedVector<float>>(std::move(freq_mag)));
    }

    // Need to reverse spectrums, because audio detector interface accepts sepctrum vector in the order of
    // from newest (largest timestamp) to oldest (smallest timestamp) in the vector.
    std::reverse(spectrums.begin(), spectrums.end());

    return test_func(spectrums, target_bool);
}




const std::map<std::string, TestFunction> TEST_MAP = {
    {"Kernels_ImageScaleBrightness", std::bind(image_void_detector_helper, test_kernels_ImageScaleBrightness, _1)},
    {"Kernels_BinaryMatrix", std::bind(image_void_detector_helper, test_kernels_BinaryMatrix, _1)},
    {"Kernels_FilterRGB32Range", std::bind(image_void_detector_helper, test_kernels_FilterRGB32Range, _1)},
    {"Kernels_FilterRGB32Euclidean", std::bind(image_void_detector_helper, test_kernels_FilterRGB32Euclidean, _1)},
    {"Kernels_ToBlackWhiteRGB32Range", std::bind(image_void_detector_helper, test_kernels_ToBlackWhiteRGB32Range, _1)},
    {"Kernels_FilterByMask", std::bind(image_void_detector_helper, test_kernels_FilterByMask, _1)},
    {"Kernels_CompressRGB32ToBinaryEuclidean", std::bind(image_void_detector_helper, test_kernels_CompressRGB32ToBinaryEuclidean, _1)},
    {"Kernels_Waterfill", std::bind(image_void_detector_helper, test_kernels_Waterfill, _1)},
    {"CommonFramework_BlackBorderDetector", std::bind(image_bool_detector_helper, test_CommonFramework_BlackBorderDetector, _1)},
    {"NintendoSwitch_UpdatePopupDetector", std::bind(image_bool_detector_helper, test_NintendoSwitch_UpdatePopupDetector, _1)},
    {"PokemonSwSh_YCommMenuDetector", std::bind(image_bool_detector_helper, test_pokemonSwSh_YCommMenuDetector, _1)},
    {"PokemonSwSh_MaxLair_BattleMenuDetector", std::bind(image_bool_detector_helper, test_pokemonSwSh_MaxLair_BattleMenuDetector, _1)},
    {"PokemonSwSh_DialogTriangleDetector", std::bind(image_bool_detector_helper, test_pokemonSwSh_DialogTriangleDetector, _1)},
    {"PokemonSwSh_RetrieveEggArrowFinder", std::bind(image_bool_detector_helper, test_pokemonSwSh_RetrieveEggArrowFinder, _1)},
    {"PokemonSwSh_YCommIconDetector", std::bind(image_bool_detector_helper, test_pokemonSwSh_YCommIconDetector, _1)},
    {"PokemonSwSh_RotomPhoneMenuArrowFinder", std::bind(image_int_detector_helper, test_pokemonSwSh_RotomPhoneMenuArrowFinder, _1)},
    {"PokemonSwSh_StoragePokemonMenuArrowFinder", std::bind(image_bool_detector_helper, test_pokemonSwSh_StoragePokemonMenuArrowFinder, _1)},
    {"PokemonSwSh_CheckNurseryArrowFinder", std::bind(image_bool_detector_helper, test_pokemonSwSh_CheckNurseryArrowFinder, _1)},
    {"PokemonSwSh_BlackDialogBoxDetector", std::bind(image_bool_detector_helper, test_pokemonSwSh_BlackDialogBoxDetector, _1)},
    {"PokemonSwSh_BoxShinySymbolDetector", std::bind(image_bool_detector_helper, test_pokemonSwSh_BoxShinySymbolDetector, _1)},
    {"PokemonSwSh_BoxGenderDetector", std::bind(image_int_detector_helper, test_pokemonSwSh_BoxGenderDetector, _1)},
    {"PokemonSwSh_SelectionArrowFinder", std::bind(image_int_detector_helper, test_pokemonSwSh_SelectionArrowFinder, _1)},
    {"PokemonLA_BattleMenuDetector", std::bind(image_bool_detector_helper, test_pokemonLA_BattleMenuDetector, _1)},
    {"PokemonLA_BattlePokemonSwitchDetector", std::bind(image_bool_detector_helper, test_pokemonLA_BattlePokemonSwitchDetector, _1)},
    {"PokemonLA_TransparentDialogueDetector", std::bind(image_bool_detector_helper, test_pokemonLA_TransparentDialogueDetector, _1)},
    {"PokemonLA_EventDialogDetector", std::bind(image_bool_detector_helper, test_pokemonLA_EventDialogDetector, _1)},
    {"PokemonLA_DialogueYellowArrowDetector", std::bind(image_bool_detector_helper, test_pokemonLA_DialogueYellowArrowDetector, _1)},
    {"PokemonLA_BlackOutDetector", std::bind(image_bool_detector_helper, test_pokemonLA_BlackOutDetector, _1)},
    {"PokemonLA_BattleStartDetector", std::bind(image_bool_detector_helper, test_pokemonLA_BattleStartDetector, _1)},
    {"PokemonLA_BerryTreeDetector", std::bind(image_void_detector_helper, test_pokemonLA_BerryTreeDetector, _1)},
    {"PokemonLA_MMOQuestionMarkDetector", std::bind(image_words_detector_helper, test_pokemonLA_MMOQuestionMarkDetector, _1)},
    {"PokemonLA_StatusInfoScreenDetector", std::bind(image_words_detector_helper, test_pokemonLA_StatusInfoScreenDetector, _1)},
    {"PokemonLA_WildPokemonFocusDetector", std::bind(image_words_detector_helper, test_pokemonLA_WildPokemonFocusDetector, _1)},
    {"PokemonLA_BattleSpriteWatcher", std::bind(image_words_detector_helper, test_pokemonLA_BattleSpriteWatcher, _1)},
    {"PokemonLA_SaveScreenDetector", std::bind(image_words_detector_helper, test_pokemonLA_SaveScreenDetector, _1)},
    {"PokemonLA_MapMarkerLocator", std::bind(image_non_negative_float_detector_helper, test_pokemonLA_MapMarkerLocator, _1)},
    {"PokemonLA_MapZoomLevelReader", std::bind(image_int_detector_helper, test_pokemonLA_MapZoomLevelReader, _1)},
    {"PokemonLA_BattleSpriteArrowDetector", std::bind(image_int_detector_helper, test_pokemonLA_BattleSpriteArrowDetector, _1)},
    {"PokemonLA_MapMissionTabReader", std::bind(image_bool_detector_helper, test_pokemonLA_MapMissionTabReader, _1)},
    {"PokemonLA_ShinySoundDetector", std::bind(sound_bool_detector_helper, test_pokemonLA_shinySoundDetector, _1)},
    {"PokemonLA_MMOSpriteMatcher", test_pokemonLA_MMOSpriteMatcher},
    {"PokemonLA_MapWeatherAndTimeReader", std::bind(image_words_detector_helper, test_pokemonLA_MapWeatherAndTimeReader, _1)},
    {"PokemonLA_FlagTrackerPerformance", std::bind(image_int_detector_helper, test_pokemonLA_FlagTracker_performance, _1)},
    {"PokemonSV_MapDetector", std::bind(image_words_detector_helper, test_pokemonSV_MapDetector, _1)},
    {"PokemonSV_PicnicDetector", std::bind(image_bool_detector_helper, test_pokemonSV_PicnicDetector, _1)},
    {"PokemonSV_TeraCardFinder", std::bind(image_bool_detector_helper, test_pokemonSV_TeraCardFinder, _1)},
    {"PokemonSV_TerastallizingDetector", std::bind(image_bool_detector_helper, test_pokemonSV_TerastallizingDetector, _1)},
    {"PokemonSV_TeraSilhouetteReader", std::bind(image_words_detector_helper, test_pokemonSV_TeraSilhouetteReader, _1)},
    {"PokemonSV_TeraTypeReader", std::bind(image_words_detector_helper, test_pokemonSV_TeraTypeReader, _1)},
    {"PokemonSV_SandwichRecipeDetector", std::bind(image_words_detector_helper, test_pokemonSV_SandwichRecipeDetector, _1)},
    {"PokemonSV_SandwichHandDetector", std::bind(image_words_detector_helper, test_pokemonSV_SandwichHandDetector, _1)},
    {"PokemonSV_BoxPokemonInfoDetector", std::bind(image_words_detector_helper, test_pokemonSV_BoxPokemonInfoDetector, _1)},
    {"PokemonSV_SomethingInBoxSlotDetector", std::bind(image_bool_detector_helper, test_pokemonSV_SomethingInBoxSlotDetector, _1)},
    {"PokemonSV_BoxEggDetector", std::bind(image_bool_detector_helper, test_pokemonSV_BoxEggDetector, _1)},
    {"PokemonSV_BoxPartyEggDetector", std::bind(image_int_detector_helper, test_pokemonSV_BoxPartyEggDetector, _1)},
    {"PokemonSV_OverworldDetector", std::bind(image_bool_detector_helper, test_pokemonSV_OverworldDetector, _1)},
    {"PokemonSV_BoxBottomButtonDetector", std::bind(image_words_detector_helper, test_pokemonSV_BoxBottomButtonDetector, _1)},
    {"PokemonSV_SandwichIngredientsDetector", std::bind(image_words_detector_helper, test_pokemonSV_SandwichIngredientsDetector, _1)},
    {"PokemonSV_SandwichIngredientReader", test_pokemonSV_SandwichIngredientReader},
    {"PokemonSV_AdvanceDialogDetector", std::bind(image_bool_detector_helper, test_pokemonSV_AdvanceDialogDetector, _1)},
    {"PokemonSV_SwapMenuDetector", std::bind(image_bool_detector_helper, test_pokemonSV_SwapMenuDetector, _1)},
    {"PokemonSV_DialogBoxDetector", std::bind(image_bool_detector_helper, test_pokemonSV_DialogBoxDetector, _1)},
    {"PokemonSV_FastTravelDetector", std::bind(image_bool_detector_helper, test_pokemonSV_FastTravelDetector, _1)},
    {"PokemonSV_MapPokeCenterIconDetector", std::bind(image_int_detector_helper, test_pokemonSV_MapPokeCenterIconDetector, _1)},
    {"PokemonSV_ESPPressedEmotionDetector", std::bind(image_bool_detector_helper, test_pokemonSV_ESPPressedEmotionDetector, _1)},
    {"PokemonSV_MapFlyMenuDetector", std::bind(image_bool_detector_helper, test_pokemonSV_MapFlyMenuDetector, _1)},
    {"PokemonSV_SandwichPlateDetector", std::bind(image_words_detector_helper, test_pokemonSV_SandwichPlateDetector, _1)},
    {"PokemonSV_RecentlyBattledDetector", std::bind(image_bool_detector_helper, test_pokemonSV_RecentlyBattledDetector, _1)},
    {"PokemonLZA_NormalDialogBoxDetector", std::bind(image_bool_detector_helper, test_pokemonZLA_NormalDialogBoxDetector, _1)},
};

TestFunction find_test_function(const std::string& test_space, const std::string& test_name){
    const auto it = TEST_MAP.find(test_space + "_" + test_name);
    if (it == TEST_MAP.end()){
        cerr << "Warning: no test object named " << test_space << "_" << test_name << " found in the code." << endl;
        return nullptr;
    }
    return it->second;
}

}
