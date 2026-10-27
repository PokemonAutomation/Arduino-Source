/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "Tests/ImageUnitTest.h"
#include "Tests/TestUtils.h"
#include "PokemonHome_ButtonDetector.h"

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

class Test_ButtonDetector : public ImageUnitTest{
public:

    Test_ButtonDetector(
        const std::string& image,
        std::optional<ButtonType> expected
    )
        : ImageUnitTest("PokemonHome::ButtonDetector - " + image, {image})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        DummyVideoOverlay video_overlay;
        const ImageRGB32& image = images[0];
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
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", ButtonType::ButtonB);
    database.add<Test_ButtonDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", ButtonType::ButtonB);

}

}
}
}
