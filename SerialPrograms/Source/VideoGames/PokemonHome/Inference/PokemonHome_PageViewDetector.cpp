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
    database.add<Test_PageViewDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", false);
}

}
}
}
