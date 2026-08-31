/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonHome_ButtonDetector.h"
#include "Tests/TestUtils.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{



class ButtonMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    // image template matcher for buttons
    // - min_width: candidate image min width if video stream is 4k
    // - min_height: candidate image min height if video stream is 4k
    ButtonMatcher(ButtonType type, size_t min_width, size_t min_height, double max_rmsd);
    
    static const ButtonMatcher& B(){
        static ButtonMatcher matcher(ButtonType::ButtonB, 25, 25, 80);
        return matcher;
    }
    static const ButtonMatcher& Plus(){
        static ButtonMatcher matcher(ButtonType::ButtonPlus, 25, 25, 80);
        return matcher;
    }
    

    virtual bool check_image(Resolution input_resolution, const ImageViewRGB32& image) const override{
        size_t min_width = m_min_width * input_resolution.width / 1920;
        size_t min_height = m_min_height * input_resolution.height / 1080;
//        cout << "???? check_image() ???? min size " << min_width << " x " << min_height
//             << " got " << image.width() << " x " << image.height() << endl;
        return image.width() >= min_width && image.height() >= min_height;
    };

    size_t m_min_width;
    size_t m_min_height;
    double m_max_rmsd;
};







const char* template_path(ButtonType type){
    switch (type){
    case ButtonType::ButtonB:
        return "PokemonHome/Buttons/ButtonB-Template.png";
    case ButtonType::ButtonPlus:
        return "PokemonHome/Buttons/ButtonPlus-Template.png";
    default:
        return "";
    }
}

const char* button_name(ButtonType type){
    switch (type){
    case ButtonType::ButtonB:
        return "ButtonB";
    case ButtonType::ButtonPlus:
        return "ButtonPlus";
    default:
        return "";
    }
}

const ButtonMatcher& get_button_matcher(ButtonType type){
    switch (type){
    case ButtonType::ButtonB:
        return ButtonMatcher::B();
    case ButtonType::ButtonPlus:
        return ButtonMatcher::Plus();
    default:
        throw std::runtime_error("No corresponding ButtonMatcher for ButtonType");
    }
}



ButtonMatcher::ButtonMatcher(ButtonType type, size_t min_width, size_t min_height, double max_rmsd)
    : WaterfillTemplateMatcher(template_path(type), COLOR_BLACK, COLOR_WHITE, 100)
    , m_min_width(min_width)
    , m_min_height(min_height)
    , m_max_rmsd(max_rmsd)
{}



ButtonDetector::ButtonDetector(
    Color color,
    ButtonType button_type,
    const ImageFloatBox& box,
    VideoOverlay* overlay
)
    : m_color(color)
    , m_matcher(get_button_matcher(button_type))
    , m_box(box)
    , m_overlay(overlay)
{

}
void ButtonDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool ButtonDetector::detect(const ImageViewRGB32& screen){

    const double screen_rel_size = (screen.height() / 1080.0);
    const double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    const double min_area_1080p = 500.0;
    const size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff000000, 0xffe0e0e0},
        {0xff000000, 0xffd0d0d0},
        {0xff000000, 0xffc0c0c0},
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        m_matcher,
        FILTERS,
        {min_area, SIZE_MAX},
        m_matcher.m_max_rmsd,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
//            cout << "width = " << object.width() << ", height = " << object.height() << endl;
            m_last_detected = translate_to_parent(screen, m_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}

class Test_ButtonDetector : public UnitTest{
public:

    Test_ButtonDetector(
        const std::string& image,
        std::optional<ButtonType> expected
    )
        : UnitTest("PokemonHome::ButtonDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        DummyVideoOverlay video_overlay;
        ImageRGB32 image(m_image);
        ImageFloatBox box(0.100, 0.956, 0.107, 0.041);
        ButtonDetector b_detector(COLOR_RED, ButtonType::ButtonB, box, &video_overlay);
        ButtonDetector plus_detector(COLOR_RED, ButtonType::ButtonPlus, box, &video_overlay);

        const bool b_detected = b_detector.detect(image);
        const bool plus_detected = plus_detector.detect(image);
        const bool expected_b = m_expected && *m_expected == ButtonType::ButtonB;
        const bool expected_plus = m_expected && *m_expected == ButtonType::ButtonPlus;

        if (b_detected == expected_b && plus_detected == expected_plus)
            return true;

        std::string received;
        if (b_detected && plus_detected){
            received = "ButtonB and ButtonPlus";
        }else if (b_detected){
            received = button_name(ButtonType::ButtonB);
        }else if (plus_detected){
            received = button_name(ButtonType::ButtonPlus);
        }else{
            received = "none";
        }

        return std::string("Expected: ") + (m_expected ? button_name(*m_expected) : "none")
            + ", received: " + received;
    };

private:
    std::string m_image;
    std::optional<ButtonType> m_expected;
};

void add_tests_ButtonDetector(UnitTestDatabase& database){
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/bidoof_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/capskid_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/castform_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/cyclizar_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/enamorus_Shiny.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/glimmet_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/gogoat_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/hatterne_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/houndstone_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/ironBunde_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/ironThorns_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/kingler_Shiny.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/komala_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/krabby_Shiny.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/machamp_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/pancham_Shiny.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/rapidash_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/rellor_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/riolu_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/scovillain_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/squirtle_Shiny.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/teddiursa_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/terapagos_regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/vulpix_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/vulpix_Shiny.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/wartortle_Regular.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/wurmple_Regular.png", ButtonType::ButtonB);
}

}
}
}
