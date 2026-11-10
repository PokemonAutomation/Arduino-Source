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
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_03.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_04.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_05.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_06.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_07.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_08.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_09.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row0_Column0_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row0_Column0_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row2_Column0_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row3_Column3_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row3_Column3_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxName_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxName_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_03.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_04.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_05.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_06.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_07.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_08.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_09.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_10.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_11.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row0_Column0_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row0_Column0_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row3_Column0_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Newest30_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Newest30_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Normal_Newest30_03.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_03.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_04.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_05.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_03.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_04.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_05.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_06.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_07.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_08.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_09.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_10.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_11.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_12.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_13.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_14.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_15.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_16.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column0_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column0_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column5_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column5_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Newest30_01.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/BoxView/SelectionArrow_Relocate_Newest30_02.png", false);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page01_Box011_Relocate_Grid_R1C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page01_Box011_Relocate_Grid_R1C4_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page01_Box027_Normal_Grid_R4C2_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page01_Box028_Normal_Grid_R4C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page01_Box029_Normal_Grid_R4C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page01_Normal_Back_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page02_Box041_Normal_Grid_R1C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page02_Box047_Relocate_Grid_R2C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page02_Box059_Normal_Grid_R4C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page02_Box059_Normal_Grid_R4C4_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page02_Box059_Relocate_Grid_R4C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page02_Box059_Relocate_Grid_R4C4_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page03_Box069_Normal_Grid_R1C2_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page03_Box071_Normal_Grid_R1C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page03_Box086_Relocate_Grid_R4C1_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page03_Multi_Back_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page03_Relocate_Back_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page03_Relocate_Back_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page03_Relocate_Back_Frame03.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box097_Multi_Grid_R1C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box105_Normal_Grid_R2C2_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box109_Multi_Grid_R3C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box117_Normal_Grid_R4C2_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box118_Normal_Grid_R4C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page04_Box119_Multi_Grid_R4C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box128_Multi_Grid_R1C1_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box129_Multi_Grid_R1C2_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box129_Multi_Grid_R1C2_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box130_Multi_Grid_R1C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box132_Normal_Grid_R1C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box134_Multi_Grid_R2C1_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box134_Multi_Grid_R2C1_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box136_Multi_Grid_R2C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box136_Multi_Grid_R2C3_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box138_Normal_Grid_R2C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box142_Multi_Grid_R3C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box148_Multi_Grid_R4C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page05_Box150_Normal_Grid_R4C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box162_Normal_Grid_R1C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box163_Relocate_Grid_R2C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box166_Multi_Grid_R2C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box172_Multi_Grid_R3C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box175_Relocate_Grid_R4C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box178_Multi_Grid_R4C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box178_Relocate_Grid_R4C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Box179_Multi_Grid_R4C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page06_Relocate_Back_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box187_Relocate_Grid_R1C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box189_Relocate_Grid_R1C2_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box192_Normal_Grid_R1C5_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box193_Normal_Grid_R2C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box195_Multi_Grid_R2C2_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box195_Relocate_Grid_R2C2_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box196_Multi_Grid_R2C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box199_Normal_Grid_R3C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box208_Relocate_Grid_R4C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Box209_Multi_Grid_R4C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page07_Multi_Back_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box223_Normal_Grid_R2C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box226_Multi_Grid_R2C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box235_Relocate_Grid_R4C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box235_Relocate_Grid_R4C0_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box235_Relocate_Grid_R4C0_Frame03.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box235_Relocate_Grid_R4C0_Frame04.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box235_Relocate_Grid_R4C0_Frame05.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box236_Relocate_Grid_R4C1_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Box238_Relocate_Grid_R4C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Relocate_Back_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page08_Relocate_Back_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box247_Normal_Grid_R1C0_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box254_Normal_Grid_R2C1_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box256_Multi_Grid_R2C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box260_Normal_Grid_R3C1_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box262_Multi_Grid_R3C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box262_Multi_Grid_R3C3_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box262_Multi_Grid_R3C3_Frame03.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box262_Multi_Grid_R3C3_Frame04.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page09_Box262_Normal_Grid_R3C3_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box281_Relocate_Grid_R1C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box285_Normal_Grid_R2C2_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box285_Normal_Grid_R2C2_Frame02.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box285_Normal_Grid_R2C2_Frame03.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box285_Normal_Grid_R2C2_Frame04.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box285_Relocate_Grid_R2C2_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/PageView/Page10_Box287_Relocate_Grid_R2C4_Frame01.png", true);
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", false);
}

}
}
}
