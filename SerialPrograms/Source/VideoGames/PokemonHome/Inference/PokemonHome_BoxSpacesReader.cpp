/*  Pokemon Home Box Spaces Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <charconv>
#include <string>
#include "Common/Cpp/Strings/StringTools.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "Tests/ImageUnitTest.h"
#include "PokemonHome_PageViewDetector.h"
#include "PokemonHome_BoxSpacesReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
namespace{

const ImageFloatBox PAGE_TITLE(0.177, 0.102, 0.158, 0.042);

}

void BoxSpacesReader::make_overlays(VideoOverlaySet& items) const{
    PageViewDetector(m_color).make_overlays(items);
    items.add(m_color, PAGE_TITLE);
}

std::optional<size_t> BoxSpacesReader::read_page_number(const ImageViewRGB32& screen) const{
    if (!PageViewDetector().detect(screen)){
        return std::nullopt;
    }
    const ImageViewRGB32 title = extract_box_reference(screen, PAGE_TITLE);
    if (!title){
        return std::nullopt;
    }

    constexpr size_t OCR_HEIGHT = 64;
    const ImageRGB32 scaled = title.scale_to(title.width() * OCR_HEIGHT / title.height(), OCR_HEIGHT);
    const std::string text = StringTools::strip(OCR::ocr_read(Language::English, scaled));

    // Read the trailing fraction. The label before it depends on the game language.
    const size_t slash = text.rfind('/');
    if (slash == std::string::npos || slash == 0){
        return std::nullopt;
    }
    const size_t before_number = text.find_last_not_of("0123456789", slash - 1);
    const size_t number_start = before_number == std::string::npos ? 0 : before_number + 1;
    const char* end = text.data() + text.size();
    size_t page = 0;
    const auto [page_end, page_error] = std::from_chars(text.data() + number_start, text.data() + slash, page);
    if (page_error != std::errc{} || page_end != text.data() + slash){
        return std::nullopt;
    }

    size_t total = 0;
    const auto [total_end, total_error] = std::from_chars(text.data() + slash + 1, end, total);
    if (total_error != std::errc{} || total_end != end || page == 0 || page > total || total > 10){
        return std::nullopt;
    }
    return page;
}

class Test_BoxSpacesReader : public ImageUnitTest{
public:
    Test_BoxSpacesReader(const std::string& image, std::optional<size_t> expected)
        : ImageUnitTest("PokemonHome::BoxSpacesReader - " + image, {image})
        , m_expected(expected)
    {
        m_memory += PADDLE_OCR_SINGLE_LINE_MEMORY_ESTIMATE;
    }

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        const auto result = BoxSpacesReader().read_page_number(images[0]);
        if (result == m_expected){
            return true;
        }
        return "Expected page " + (m_expected ? std::to_string(*m_expected) : "none")
            + ", read " + (result ? std::to_string(*result) : "none");
    }

private:
    std::optional<size_t> m_expected;
};

void add_tests_BoxSpacesReader(UnitTestDatabase& database){
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page01_Box011_Multi_Grid_R1C4_Frame01.png", 1);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page02_Box037_Multi_Grid_R1C0_Frame01.png", 2);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page02_Box040_Multi_Grid_R1C3_Frame01.png", 2);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page03_Box067_Multi_Grid_R1C0_Frame01.png", 3);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page03_Box073_Multi_Grid_R2C0_Frame01.png", 3);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page04_Box096_Relocate_Grid_R0C5_Frame01.png", 4);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page04_Box103_Multi_Grid_R2C0_Frame01.png", 4);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page04_Box108_Relocate_Grid_R2C5_Frame01.png", 4);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page04_Box108_Relocate_Grid_R2C5_Frame02.png", 4);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page04_Box115_Multi_Grid_R4C0_Frame01.png", 4);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page05_Box126_Relocate_Grid_R0C5_Frame01.png", 5);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page05_Multi_Back_Frame01.png", 5);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page05_Relocate_BoxName_Frame01.png", 5);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page06_Box156_Relocate_Grid_R0C5_Frame01.png", 6);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page06_Box157_Relocate_Grid_R1C0_Frame01.png", 6);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page06_Box157_Relocate_Grid_R1C0_Frame02.png", 6);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page06_Box162_Normal_Grid_R1C5_Frame01.png", 6);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page06_Box165_Relocate_Grid_R2C2_Frame01.png", 6);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page06_Box172_Relocate_Grid_R3C3_Frame01.png", 6);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page06_Multi_Back_Frame01.png", 6);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page06_Relocate_Back_Frame01.png", 6);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page06_Relocate_BoxName_Frame01.png", 6);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page07_Box192_Normal_Grid_R1C5_Frame01.png", 7);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page07_Box198_Normal_Grid_R2C5_Frame01.png", 7);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page07_Box198_Normal_Grid_R2C5_Frame02.png", 7);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page07_Box208_Relocate_Grid_R4C3_Frame01.png", 7);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page08_Box234_Normal_Grid_R3C5_Frame01.png", 8);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page08_Box238_Relocate_Grid_R4C3_Frame01.png", 8);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page08_Box239_Normal_Grid_R4C4_Frame01.png", 8);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page08_Box240_Normal_Grid_R4C5_Frame01.png", 8);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page09_Box251_Normal_Grid_R1C4_Frame01.png", 9);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page09_Box251_Normal_Grid_R1C4_Frame02.png", 9);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page09_Box257_Normal_Grid_R2C4_Frame01.png", 9);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page09_Box269_Normal_Grid_R4C4_Frame01.png", 9);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page09_Normal_Back_Frame01.png", 9);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page09_Normal_Back_Frame02.png", 9);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page09_Normal_BoxName_Frame01.png", 9);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page10_Box281_Multi_Grid_R1C4_Frame01.png", 10);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page10_Box281_Multi_Grid_R1C4_Frame02.png", 10);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page10_Box287_Normal_Grid_R2C4_Frame01.png", 10);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page10_Box293_Normal_Grid_R3C4_Frame01.png", 10);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page10_Box293_Normal_Grid_R3C4_Frame02.png", 10);

    // Holding a box replaces the Quit prompt that the page-view check requires.
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page01_Box022_Relocate_Grid_R3C3_Frame01.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page02_Box059_Relocate_Grid_R4C4_Frame01.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page02_Box060_Relocate_Grid_R4C5_Frame01.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page03_Box090_Relocate_Grid_R4C5_Frame01.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page04_Box113_Relocate_Grid_R3C4_Frame01.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page04_Box113_Relocate_Grid_R3C4_Frame02.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page10_Box274_Relocate_Grid_R0C3_Frame01.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page10_Box274_Relocate_Grid_R0C3_Frame02.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page10_Box292_Relocate_Grid_R3C3_Frame01.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page10_Box292_Relocate_Grid_R3C3_Frame02.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page10_Box297_Relocate_Grid_R4C2_Frame01.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page10_Box298_Relocate_Grid_R4C3_Frame01.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/PageView/Page10_Relocate_Back_Frame01.png", std::nullopt);

    database.add<Test_BoxSpacesReader>("PokemonHome/BoxView/BoxView-1.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxName_01.png", std::nullopt);
    database.add<Test_BoxSpacesReader>("PokemonHome/SummaryScreen/annihilape_Regular.png", std::nullopt);
}

}
}
}
