/*  Test Map
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "PokemonLA_Tests.h"
#include "TestMap.h"
#include "TestUtils.h"

#include <QImage>
#include <QImageReader>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

using namespace std::placeholders;  // for _1, _2, _3... with std::bind()

namespace PokemonAutomation{


// For each test object, TestFunction will do the work of skipping non-test
// files, read data from test file names or gold files and possibly more.
// To reuse the code, we design the following helper functions.

using ScreenDetectorFunction = std::function<int(const QImage& image, bool target)>;


// Helper for testing screen detectors, which return true or false based on
// screen content.
// The target test result (whether this test file should be detected as true or false)
// is stored as part of the filename. For example, IngoBattleDayTime-True.png.
int screen_detector_helper(ScreenDetectorFunction test_func, const std::string& test_path){
    const QString file_path = QString::fromStdString(test_path);
    QImageReader reader(file_path);
    if (reader.canRead() == false){
        cout << "Skip " << test_path << " as it cannot be read as image" << endl;
        return -1;
    }

    // Search for the target test result from test filename.
    // const QFileInfo file_info(QString::fromStdString(test_path));
    // const std::string filename = file_info.fileName().toStdString();
    const size_t target_pos = test_path.rfind('.');
    if (target_pos == std::string::npos){
        cerr << "Error: image test file " << test_path << " has no \".\" in the filename." << endl;
        return 1;
    }

    const auto test_path_base = QString::fromStdString(test_path.substr(0, target_pos));
    bool target_bool = false;
    if (test_path_base.endsWith("-True")){
        target_bool = true;
    } else if (test_path_base.endsWith("-False")){
        target_bool = false;
    } else{
        cerr << "Error: image test file " << test_path << " has incorrect target detection result (-True/-False) set in the filename." << endl;
        return 1;
    }

    cout << "Test file: " << test_path << endl;
    const QImage image = reader.read();

    return test_func(image, target_bool);
}

const std::map<std::string, TestFunction> TEST_MAP = {
    {"PokemonLA_BattleMenuDetector", std::bind(screen_detector_helper, test_pokemonLA_BattleMenuDetector, _1)},
    {"PokemonLA_BattlePokemonSwitchDetector", std::bind(screen_detector_helper, test_pokemonLA_BattlePokemonSwitchDetector, _1)},
    {"PokemonLA_DialogueEllipseDetector", std::bind(screen_detector_helper, test_pokemonLA_DialogueEllipseDetector, _1)}
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