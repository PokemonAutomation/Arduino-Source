/*  Selection Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "Tests/TestUtils.h"
#include "PokemonHome_SelectionArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class SelectionArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    // The Pokemon Home arrows are red on a light background.
    SelectionArrowMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(0xffa00000), Color(0xffffc0c0), 100
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const SelectionArrowMatcher& matcher(SelectionArrowType type){
        switch (type){
        case SelectionArrowType::RIGHT:
            return RIGHT_ARROW();
        case SelectionArrowType::DOWN:
            return DOWN_ARROW();
        default:
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid enum.");
        }
    }
    static const SelectionArrowMatcher& RIGHT_ARROW(){
        static SelectionArrowMatcher matcher("PokemonHome/SelectionArrows/SelectionArrowRight.png");
        return matcher;
    }
    static const SelectionArrowMatcher& DOWN_ARROW(){
        static SelectionArrowMatcher matcher("PokemonHome/SelectionArrows/SelectionArrowDown.png");
        return matcher;
    }
};

SelectionArrowDetector::SelectionArrowDetector(
    Color color,
    VideoOverlay* overlay,
    SelectionArrowType type,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_type(type)
    , m_arrow_box(box)
{}
void SelectionArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool SelectionArrowDetector::detect(const ImageViewRGB32& screen){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 700;
    double rmsd_threshold = 80;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xff800000, 0xffff9090},
        {0xffa00000, 0xffffa0a0},
        {0xffc00000, 0xffffc0c0},
        {0xffd00000, 0xffffd0d0},
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_arrow_box),
        SelectionArrowMatcher::matcher(m_type),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(screen, m_arrow_box, object);
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

class Test_SelectionArrowDetector : public UnitTest{
public:

    Test_SelectionArrowDetector(
        const std::string& image,
        std::optional<SelectionArrowType> expected
    )
        : UnitTest("PokemonHome::SelectionArrowDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        DummyVideoOverlay video_overlay;
        ImageRGB32 image(m_image);
        SelectionArrowDetector right_detector(COLOR_BLACK, &video_overlay, SelectionArrowType::RIGHT, ImageFloatBox(0.463, 0.09, 0.04, 0.06));
        SelectionArrowDetector down_detector(COLOR_BLACK, &video_overlay, SelectionArrowType::DOWN, ImageFloatBox(0.463, 0.09, 0.04, 0.06));

        const bool right_detected = right_detector.detect(image);
        const bool down_detected = down_detector.detect(image);

        const bool expected_right = m_expected && *m_expected == SelectionArrowType::RIGHT;
        const bool expected_down = m_expected && *m_expected == SelectionArrowType::DOWN;

        if (right_detected == expected_right && down_detected == expected_down)
            return true;

        auto arrow_name = [](std::optional<SelectionArrowType> arrow){
            if (!arrow)
                return std::string("none");
            return *arrow == SelectionArrowType::RIGHT
                ? std::string("RIGHT")
                : std::string("DOWN");
        };

        std::optional<SelectionArrowType> result;
        if (right_detected && down_detected){
            return "Expected: " + arrow_name(m_expected) + ", received: RIGHT and DOWN";
        }else if (right_detected){
            result = SelectionArrowType::RIGHT;
        }else if (down_detected){
            result = SelectionArrowType::DOWN;
        }

        return "Expected: " + arrow_name(m_expected) + ", received: " + arrow_name(result);
    };

private:
    std::string m_image;
    std::optional<SelectionArrowType> m_expected;
};

void add_tests_SelectionArrowDetector(UnitTestDatabase& database){
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/bidoof_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/capskid_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/castform_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/cyclizar_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/enamorus_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/glimmet_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gogoat_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/hatterne_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/houndstone_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ironBunde_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ironThorns_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/kingler_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/komala_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/krabby_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/machamp_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/pancham_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rapidash_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rellor_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/riolu_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/scovillain_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/squirtle_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/teddiursa_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/terapagos_regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/vulpix_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/vulpix_Shiny.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/wartortle_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/wurmple_Regular.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", std::nullopt);
    database.add<Test_SelectionArrowDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", std::nullopt);
}

}
}
}