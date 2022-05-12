/*  Test Map
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "PokemonLA_Tests.h"
#include "TestMap.h"
#include "TestUtils.h"

#include <QFileInfo>
#include <QImage>
#include <QImageReader>

#include <iostream>
#include <sstream>
using std::cout;
using std::cerr;
using std::endl;

using namespace std::placeholders;  // for _1, _2, _3... with std::bind()

namespace PokemonAutomation{


// For each test object, TestFunction will do the work of skipping non-test
// files, read data from test file names or gold files and possibly more.
// To reuse the code, we design the following helper functions.

using ImageFilenameFunction = std::function<int(const QImage& image, const std::string& filename_base)>;

using ImageBoolDetectorFunction = std::function<int(const QImage& image, bool target)>;

using ImageKeywordsDetectorFunction = std::function<int(const QImage& image, const std::vector<std::string>& keywords)>;

using ImageVoidDetectorFunction = std::function<void(const QImage& image)>;

// Basic check on whether an image can be loaded.
// Also strip the image format suffix (.png and so on)

// Helper for testing code that reads an image and uses filename to get the target outcome for the code.
// test_func: reads an image and the image filename base and returns an int code.
int image_filename_detector_helper(const std::string& test_path, ImageFilenameFunction test_func){
    const QString file_path = QString::fromStdString(test_path);
    QImageReader reader(file_path);
    if (reader.canRead() == false){
        cout << "Skip " << test_path << " as it cannot be read as image" << endl;
        return -1;
    }

    QFileInfo file_info(file_path);
    std::string filename = file_info.fileName().toStdString();

    // Search for the target test result from test filename.
    const size_t target_pos = filename.rfind('.');
    if (target_pos == std::string::npos){
        cerr << "Error: image test file " << test_path << " has no \".\" in the filename." << endl;
        return 1;
    }

    // cout << "Test file: " << test_path << endl;

    const auto filename_base = filename.substr(0, target_pos);

    const QImage image = reader.read();
    return test_func(image, filename_base);
}


// Helper for testing detector code that reads an image and returns true or false.
// The target test result (whether this test file should be detected as true or false)
// is stored as part of the filename. For example, IngoBattleDayTime-True.png.
int image_bool_detector_helper(ImageBoolDetectorFunction test_func, const std::string& test_path){
    auto parse_filename_and_run_test = [&](const QImage& image, const std::string& filename_base){
        const auto name_base = QString::fromStdString(filename_base);
        bool target_bool = false;
        if (name_base.endsWith("-True")){
            target_bool = true;
        } else if (name_base.endsWith("-False")){
            target_bool = false;
        } else{
            cerr << "Error: image test file " << test_path << " has incorrect target detection result (-True/-False) set in the filename." << endl;
            return 1;
        }

        return test_func(image, target_bool);
    };

    return image_filename_detector_helper(test_path, parse_filename_and_run_test);
}

// Helper for testing detector code that reads an image and returns some custom data that can be described
// by keywords included in the test filename.
// The helper will split the filename by "-" into keywords and send it in the same order to the test function.
int image_keywords_detector_helper(ImageKeywordsDetectorFunction test_func, const std::string& test_path){
    auto parse_filename_and_run_test = [&](const QImage& image, const std::string& filename_base){
        const auto name_base = QString::fromStdString(filename_base);

        std::vector<std::string> keywords;
        std::istringstream is(filename_base);
        std::string keyword;
        while (getline(is, keyword, '-')){
            keywords.push_back(keyword);
        }

        return test_func(image, keywords);
    };

    return image_filename_detector_helper(test_path, parse_filename_and_run_test);
}


// Helper for testing sdetector code that reads an image and returns nothing.
// This is used for developing visual inference code where the developer writes custom
// debugging output. So no need to get target values from the test framework.
int image_void_detector_helper(ImageVoidDetectorFunction test_func, const std::string& test_path){
const QString file_path = QString::fromStdString(test_path);
    auto run_test = [&](const QImage& image, const std::string&) -> int{
        test_func(image);
        return 0;
    };

    return image_filename_detector_helper(test_path, run_test);
}





const std::map<std::string, TestFunction> TEST_MAP = {
    {"PokemonLA_BattleMenuDetector", std::bind(image_bool_detector_helper, test_pokemonLA_BattleMenuDetector, _1)},
    {"PokemonLA_BattlePokemonSwitchDetector", std::bind(image_bool_detector_helper, test_pokemonLA_BattlePokemonSwitchDetector, _1)},
    {"PokemonLA_TransparentDialogueDetector", std::bind(image_bool_detector_helper, test_pokemonLA_TransparentDialogueDetector, _1)},
    {"PokemonLA_BerryTreeDetector", std::bind(image_void_detector_helper, test_pokemonLA_BerryTreeDetector, _1)},
    {"PokemonLA_StatusInfoScreenDetector", std::bind(image_keywords_detector_helper, test_pokemonLA_StatusInfoScreenDetector, _1)}
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