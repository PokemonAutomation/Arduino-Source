/*  Y-Comm Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonSwSh_YCommDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



YCommMenuDetector::YCommMenuDetector(bool is_on)
    : m_is_on(is_on)
    , m_top(0.600, 0.020, 0.100, 0.040)
    , m_bottom(0.100, 0.970, 0.400, 0.020)
{}
void YCommMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_top);
    items.add(COLOR_RED, m_bottom);
}

bool YCommMenuDetector::detect(const ImageViewRGB32& screen){
    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
//    cout << bottom.average << bottom.stddev << endl;
    if (!is_black(bottom)){
        return !m_is_on;
    }

    ImageStats top = image_stats(extract_box_reference(screen, m_top));
//    cout << top.average << top.stddev << endl;
    if (!is_solid(top, {0.0819777, 0.124031, 0.793991}, 0.30)){
        return !m_is_on;
    }

    return m_is_on;
}



class YCommIconMatcherOffline : public ImageMatch::WaterfillTemplateMatcher{
public:
    YCommIconMatcherOffline()
        : WaterfillTemplateMatcher(
            "PokemonSwSh/YComm-Offline.png",
            Color(0, 0, 255), Color(100, 100, 255), 50
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.2;
    }
    static const YCommIconMatcherOffline& instance(){
        static YCommIconMatcherOffline matcher;
        return matcher;
    }
};
class YCommIconMatcherOnline : public ImageMatch::WaterfillTemplateMatcher{
public:
    YCommIconMatcherOnline()
        : WaterfillTemplateMatcher(
            "PokemonSwSh/YComm-Online.png",
            Color(200, 200, 200), Color(255, 255, 255), 50
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.9;
        m_area_ratio_upper = 1.2;
    }
    static const YCommIconMatcherOnline& instance(){
        static YCommIconMatcherOnline matcher;
        return matcher;
    }
};



YCommIconDetector::YCommIconDetector(Color color, bool is_on)
    : m_color(color)
    , m_icon(0.007, 0.944, 0.032, 0.054)
//    , m_left(0.004362, 0.961165, 0.007634, 0.025243)
//    , m_right(0.038000, 0.961165, 0.007634, 0.025243)
    , m_is_on(is_on)
{}

void YCommIconDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_icon);
//    items.add(COLOR_RED, m_left);
//    items.add(COLOR_RED, m_right);
}

bool YCommIconDetector::detect(const ImageViewRGB32& screen){
    const size_t MIN_SIZE = (size_t)(350. * screen.total_pixels() / (1920 * 1080.));

    {
        const std::vector<std::pair<uint32_t, uint32_t>> filters = {
            {combine_rgb(0, 0, 150), combine_rgb(100, 100, 255)},
            {combine_rgb(0, 0, 150), combine_rgb(127, 127, 255)},
            {combine_rgb(0, 0, 150), combine_rgb(191, 191, 255)},
        };
        if (match_template_by_waterfill(
            screen.size(),
            extract_box_reference(screen, m_icon),
            YCommIconMatcherOffline::instance(),
            filters,
            {MIN_SIZE, SIZE_MAX},
            120,
            [](Kernels::Waterfill::WaterfillObject& object) -> bool { return true; }
        )){
//            cout << "waterfill 0" << endl;
            return m_is_on;
        }
    }
    {
        const std::vector<std::pair<uint32_t, uint32_t>> filters = {
            {0xff808080, 0xffffffff},
            {0xffa0a0a0, 0xffffffff},
            {0xffc0c0c0, 0xffffffff},
        };
        if (match_template_by_waterfill(
            screen.size(),
            extract_box_reference(screen, m_icon),
            YCommIconMatcherOnline::instance(),
            filters,
            {MIN_SIZE, SIZE_MAX},
            120,
            [](Kernels::Waterfill::WaterfillObject& object) -> bool { return true; }
        )){
//            cout << "waterfill 1" << endl;
            return m_is_on;
        }
    }

#if 0
    ImageStats left = image_stats(extract_box_reference(screen, m_left));
    if (!is_solid(left, {0.0410557, 0.211144, 0.747801}, 0.20)){
        cout << "not solid" << endl;
        return !m_is_on;
    }
    ImageStats right = image_stats(extract_box_reference(screen, m_right));
    if (!is_solid(right, {0.0410557, 0.211144, 0.747801}, 0.20)){
        cout << "not solid" << endl;
        return !m_is_on;
    }
#endif

//    cout << "end" << endl;
    return !m_is_on;
}








class Test_YCommIconDetector : public UnitTest{
public:
    Test_YCommIconDetector(
        const std::string& resource_path,
        const std::string& image,
        bool expected_result
    )
        : UnitTest("NintendoSwitch::PokemonSwSh::YCommIconDetector - " + image)
        , m_image(resource_path + image)
        , m_expected_result(expected_result)
    {}

    template <class... Args>
    static std::unique_ptr<UnitTest> make(Args&&... args){
        return std::make_unique<Test_YCommIconDetector>(std::forward<Args>(args)...);
    }

    virtual UnitTestResult run(CancellableScope& scope) const override{
        YCommIconDetector detector(COLOR_RED, true);
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected_result;
    };

private:
    std::string m_image;
    bool m_expected_result;
};

class Test_YCommMenuDetector : public UnitTest{
public:
    Test_YCommMenuDetector(
        const std::string& resource_path,
        const std::string& image,
        bool expected_result
    )
        : UnitTest("NintendoSwitch::PokemonSwSh::YCommMenuDetector - " + image)
        , m_image(resource_path + image)
        , m_expected_result(expected_result)
    {}

    template <class... Args>
    static std::unique_ptr<UnitTest> make(Args&&... args){
        return std::make_unique<Test_YCommMenuDetector>(std::forward<Args>(args)...);
    }

    virtual UnitTestResult run(CancellableScope& scope) const override{
        YCommMenuDetector detector(true);
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected_result;
    };

private:
    std::string m_image;
    bool m_expected_result;
};




const std::vector<std::shared_ptr<const UnitTest>>& get_tests_YCommDetector(const std::string& resource_path){
    static std::vector<std::shared_ptr<const UnitTest>> TESTS{
        Test_YCommIconDetector::make(resource_path, "PokemonSwSh/YCommIconDetector/Front_Door_2_True.png", true),
        Test_YCommIconDetector::make(resource_path, "PokemonSwSh/YCommIconDetector/Front_Door_3_True.png", true),
        Test_YCommIconDetector::make(resource_path, "PokemonSwSh/YCommIconDetector/Front_Door_True.png", true),
        Test_YCommIconDetector::make(resource_path, "PokemonSwSh/YCommIconDetector/macOS_bright/Nursery_False.png", false),
        Test_YCommIconDetector::make(resource_path, "PokemonSwSh/YCommIconDetector/macOS_bright/Nursery_True.png", true),

        Test_YCommMenuDetector::make(resource_path, "PokemonSwSh/YCommMenuDetector/Avermedia_Qt5_True.jpg", true),
        Test_YCommMenuDetector::make(resource_path, "PokemonSwSh/YCommMenuDetector/MiraBox_Qt6_True.jpg", true),
        Test_YCommMenuDetector::make(resource_path, "PokemonSwSh/YCommMenuDetector/MyPin_Qt5_True.jpg", true),
        Test_YCommMenuDetector::make(resource_path, "PokemonSwSh/YCommMenuDetector/MyPin_Qt6_True.jpg", true),
        Test_YCommMenuDetector::make(resource_path, "PokemonSwSh/YCommMenuDetector/NoBrand_Qt6_True.jpg", true),
        Test_YCommMenuDetector::make(resource_path, "PokemonSwSh/YCommMenuDetector/ShadowCast_Qt6_True.jpg", true),
    };
    return TESTS;
}















}
}
}
