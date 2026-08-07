/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "Tests/TestUtils.h"
#include "PokemonSV_MapDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


namespace{

class MapOrangleFixedViewArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MapOrangleFixedViewArrowMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Map/OrangleFixedView-Template.png", Color(50,50,0), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;
    }

    static const MapOrangleFixedViewArrowMatcher& instance(){
        static MapOrangleFixedViewArrowMatcher matcher;
        return matcher;
    }
};


class MapOrangleRotatedViewArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MapOrangleRotatedViewArrowMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Map/OrangleRotatedView-Template.png", Color(50,50,0), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;
    }

    static const MapOrangleRotatedViewArrowMatcher& instance(){
        static MapOrangleRotatedViewArrowMatcher matcher;
        return matcher;
    }
};


} // end anonymous namespace


ImageFloatBox MAP_READABLE_AREA{0.197, 0.182, 0.678, 0.632};

MapFixedViewDetector::MapFixedViewDetector(Color color)
    : m_color(color)
    , m_arrow_box(0.165, 0.712, 0.014, 0.040)
{}
void MapFixedViewDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool MapFixedViewDetector::detect(const ImageViewRGB32& frame){
     const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(100, 100, 0), combine_rgb(255, 255, 200)}
    };

    const double screen_rel_size = (frame.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * 150.0);
    
    const bool detected = match_template_by_waterfill(
        frame.size(),
        extract_box_reference(frame, m_arrow_box), 
        MapOrangleFixedViewArrowMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        120,
        [](Kernels::Waterfill::WaterfillObject& object) -> bool { return true; }
    );
    
    return detected;
}


MapRotatedViewDetector::MapRotatedViewDetector(Color color)
    : m_color(color)
    , m_arrow_box(0.157, 0.720, 0.029, 0.039)
{}
void MapRotatedViewDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool MapRotatedViewDetector::detect(const ImageViewRGB32& frame){
     const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(100, 100, 0), combine_rgb(255, 255, 200)}
    };

    const double screen_rel_size = (frame.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * 450.0);
    
    const bool detected = match_template_by_waterfill(
        frame.size(),
        extract_box_reference(frame, m_arrow_box), 
        MapOrangleRotatedViewArrowMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        120,
        [](Kernels::Waterfill::WaterfillObject& object) -> bool { return true; }
    );
    
    return detected;
}


MapWatcher::MapWatcher(Color color)
: VisualInferenceCallback("MapWatcher")
, m_exit_watcher(COLOR_RED, WhiteButton::ButtonY, {0.800, 0.118, 0.030, 0.060}), m_fixed_view_watcher(color), m_rotated_view_watcher(color){}

void MapWatcher::make_overlays(VideoOverlaySet& items) const{
    m_exit_watcher.make_overlays(items);
    m_fixed_view_watcher.make_overlays(items);
    m_rotated_view_watcher.make_overlays(items);
}

bool MapWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    const bool exit_found = m_exit_watcher.process_frame(frame, timestamp);
    if (!exit_found){
        return false;
    }

    const bool fixed_found = m_fixed_view_watcher.process_frame(frame, timestamp);
    const bool rotated_found = m_rotated_view_watcher.process_frame(frame, timestamp);

    if (fixed_found && !rotated_found){
        m_in_fixed_view = true;
        return true;
    }else if (!fixed_found && rotated_found){
        m_in_fixed_view = false;
        return true;
    }

    return false;
}







class Test_MapDetector : public UnitTest{
public:
    Test_MapDetector(
        const std::string& image,
        std::vector<std::string> words
    )
        : UnitTest("PokemonSV::MapDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_words(std::move(words))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        // two words: <Map ready to exit> <In fixed view (instead of the rotated view)>
        if (m_words.size() < 2){
            std::stringstream ss;
            ss << "Error: not enough number of words in the filename. Found only " << m_words.size() << "." << std::endl;
            return ss.str();
        }

        bool target_map_existence = false;
        if (parse_bool(m_words[m_words.size()-2], target_map_existence) == false){
            std::stringstream ss;
            ss << "Error: True/False word " << m_words[m_words.size()-2] << " is wrong. Must be \"True\" or \"False\"." << std::endl;
            return ss.str();
        }

        bool target_is_fixed_view = false;
        if (parse_bool(m_words[m_words.size()-1], target_is_fixed_view) == false){
            std::stringstream ss;
            ss << "Error: True/False word " << m_words[m_words.size()-1] << " is wrong. Must be \"True\" or \"False\"." << std::endl;
            return ss.str();
        }

        WhiteButtonDetector map_exit_detector(COLOR_RED, WhiteButton::ButtonY, {0.800, 0.118, 0.030, 0.060});
        ImageRGB32 image(m_image);
        bool result_map = map_exit_detector.detect(image);

        TEST_RESULT_EQUAL(result_map, target_map_existence);

        if (result_map){

            MapFixedViewDetector map_fixed_view_detector;
            MapRotatedViewDetector map_rotated_view_detected;

            bool result_fixed = map_fixed_view_detector.detect(image);
            bool result_rotated = map_rotated_view_detected.detect(image);

            TEST_RESULT_EQUAL(result_fixed, target_is_fixed_view);
            TEST_RESULT_EQUAL(result_rotated, !target_is_fixed_view);
        }

        return true;
    };

private:
    std::string m_image;
    const std::vector<std::string> m_words;
};



void add_tests_MapDetector(UnitTestDatabase& database){
    database.add<Test_MapDetector>(
        "PokemonSV/MapDetector/BugTypeCase_False_False.png",
        std::vector<std::string>{"BugTypeCase", "False", "False"}
    );
    database.add<Test_MapDetector>(
        "PokemonSV/MapDetector/EastAreaWatchTowerAfterDLC1_True_True.png",
        std::vector<std::string>{"EastAreaWatchTowerAfterDLC1", "True", "True"}
    );
    database.add<Test_MapDetector>(
        "PokemonSV/MapDetector/EeveeCase_True_True.png",
        std::vector<std::string>{"EeveeCase", "True", "True"}
    );
    database.add<Test_MapDetector>(
        "PokemonSV/MapDetector/FuecocoCase_True_True.png",
        std::vector<std::string>{"FuecocoCase", "True", "True"}
    );
    database.add<Test_MapDetector>(
        "PokemonSV/MapDetector/GlitteratiCase_True_True.png",
        std::vector<std::string>{"GlitteratiCase", "True", "True"}
    );
    database.add<Test_MapDetector>(
        "PokemonSV/MapDetector/PoketchCase_False_True.png",
        std::vector<std::string>{"PoketchCase", "False", "True"}
    );
    database.add<Test_MapDetector>(
        "PokemonSV/MapDetector/PortoMarinada_True_False.png",
        std::vector<std::string>{"PortoMarinada", "True", "False"}
    );
    database.add<Test_MapDetector>(
        "PokemonSV/MapDetector/ZeroGate_True_True.png",
        std::vector<std::string>{"ZeroGate", "True", "True"}
    );
    database.add<Test_MapDetector>(
        "PokemonSV/MapDetector/macOS/Cascarraf_Restaurant_True_True.png",
        std::vector<std::string>{"Cascarraf_Restaurant", "True", "True"}
    );
    database.add<Test_MapDetector>(
        "PokemonSV/MapDetector/macOS/cascarrafa_True_True.png",
        std::vector<std::string>{"cascarrafa", "True", "True"}
    );
    database.add<Test_MapDetector>(
        "PokemonSV/MapDetector/macOS/cascarrafa_Zooming_False_False.png",
        std::vector<std::string>{"cascarrafa", "Zooming", "False", "False"}
    );
    database.add<Test_MapDetector>(
        "PokemonSV/MapDetector/macOS/KitakamiOniMountain_True_False.png",
        std::vector<std::string>{"KitakamiOniMountain", "True", "False"}
    );
}







}
}
}
