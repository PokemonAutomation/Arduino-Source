/*  Pokemon Home Page View Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Tests/ImageUnitTest.h"
#include "PokemonHome_StorageView.h"
#include "PokemonHome_PageViewDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

PageViewDetector::PageViewDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_button_plus_detector(COLOR_BLACK, ButtonType::ButtonPlus, STORAGE_PLUS_BUTTON, overlay)
{}

void PageViewDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, STORAGE_HEADER_STRIP);
    m_button_plus_detector.make_overlays(items);
}

bool PageViewDetector::detect(const ImageViewRGB32& screen){
    if (classify_storage_header(screen) != StorageViewHeader::PageView){
        m_button_plus_detector.reset_state();
        return false;
    }
    return m_button_plus_detector.detect(screen);
}

class Test_PageViewDetector : public ImageUnitTest{
public:
    Test_PageViewDetector(const std::string& image, bool expected)
        : ImageUnitTest("PokemonHome::PageViewDetector - " + image, {image})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        if (PageViewDetector().detect(images[0]) != m_expected){
            return m_expected ? "Expected Page View." : "Unexpected Page View detection.";
        }
        return true;
    }

private:
    bool m_expected;
};

void add_tests_PageViewDetector(UnitTestDatabase& database){
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/BoxView-1.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxName_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row2_Column1_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row2_Column1_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row3_Column2_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row3_Column3_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row3_Column4_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row4_Column4_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row4_Column4_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Newest30_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxName_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row0_Column1_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row0_Column1_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row0_Column1_03.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row1_Column1_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row1_Column4_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row1_Column4_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row2_Column1_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row2_Column3_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row2_Column4_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row3_Column2_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row3_Column2_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Newest30_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row0_Column0_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row0_Column1_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row0_Column2_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row0_Column2_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row1_Column4_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column2_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column5_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column5_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row3_Column1_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row3_Column4_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row4_Column1_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row4_Column1_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row4_Column4_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Newest30_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Newest30_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row0_Column2_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row1_Column2_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row2_Column0_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row2_Column1_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Multi_Grid_Row3_Column2_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row0_Column1_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row0_Column1_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row0_Column4_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row0_Column4_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row0_Column4_03.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Normal_Grid_Row2_Column4_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row1_Column4_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row2_Column4_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionHand_Relocate_Grid_Row2_Column4_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page01_Box011_Multi_Grid_R1C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page02_Box037_Multi_Grid_R1C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page02_Box040_Multi_Grid_R1C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page03_Box067_Multi_Grid_R1C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page03_Box073_Multi_Grid_R2C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box096_Relocate_Grid_R0C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box103_Multi_Grid_R2C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box108_Relocate_Grid_R2C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box108_Relocate_Grid_R2C5_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box115_Multi_Grid_R4C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box126_Relocate_Grid_R0C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Multi_Back_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Relocate_BoxName_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box156_Relocate_Grid_R0C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box157_Relocate_Grid_R1C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box157_Relocate_Grid_R1C0_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box162_Normal_Grid_R1C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box165_Relocate_Grid_R2C2_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box172_Relocate_Grid_R3C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Multi_Back_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Relocate_Back_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Relocate_BoxName_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box192_Normal_Grid_R1C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box198_Normal_Grid_R2C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box198_Normal_Grid_R2C5_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box208_Relocate_Grid_R4C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box234_Normal_Grid_R3C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box238_Relocate_Grid_R4C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box239_Normal_Grid_R4C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box240_Normal_Grid_R4C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box251_Normal_Grid_R1C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box251_Normal_Grid_R1C4_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box257_Normal_Grid_R2C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box269_Normal_Grid_R4C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Normal_Back_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Normal_Back_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Normal_BoxName_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box281_Multi_Grid_R1C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box281_Multi_Grid_R1C4_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box287_Normal_Grid_R2C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box293_Normal_Grid_R3C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box293_Normal_Grid_R3C4_Frame02.png", true);
    // Holding a box replaces the Quit prompt with Cancel.
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page01_Box022_Relocate_Grid_R3C3_Frame01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page02_Box059_Relocate_Grid_R4C4_Frame01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page02_Box060_Relocate_Grid_R4C5_Frame01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page03_Box090_Relocate_Grid_R4C5_Frame01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box113_Relocate_Grid_R3C4_Frame01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box113_Relocate_Grid_R3C4_Frame02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box274_Relocate_Grid_R0C3_Frame01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box274_Relocate_Grid_R0C3_Frame02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box292_Relocate_Grid_R3C3_Frame01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box292_Relocate_Grid_R3C3_Frame02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box297_Relocate_Grid_R4C2_Frame01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box298_Relocate_Grid_R4C3_Frame01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Relocate_Back_Frame01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/bidoof_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/capskid_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/castform_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/cyclizar_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/enamorus_Shiny.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/glimmet_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/gogoat_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/hatterne_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/houndstone_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/ironBunde_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/ironThorns_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/kingler_Shiny.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/komala_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/krabby_Shiny.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/machamp_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/pancham_Shiny.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/rapidash_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/rellor_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/riolu_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/scovillain_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/squirtle_Shiny.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/teddiursa_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/terapagos_regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/vulpix_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/vulpix_Shiny.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/wartortle_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/wurmple_Regular.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", false);
}

}
}
}
