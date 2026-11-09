/*  Pokemon Home Box Number Reader
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
#include "PokemonHome_BoxNumberReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
namespace{

const ImageFloatBox BOX_NUMBER(0.080, 0.718, 0.082, 0.039);

}

void BoxNumberReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, BOX_NUMBER);
}

std::optional<size_t> BoxNumberReader::read_box_number(const ImageViewRGB32& screen) const{
    const ImageViewRGB32 crop = extract_box_reference(screen, BOX_NUMBER);
    if (!crop){
        return std::nullopt;
    }

    constexpr size_t OCR_HEIGHT = 64;
    const ImageRGB32 scaled = crop.scale_to(crop.width() * OCR_HEIGHT / crop.height(), OCR_HEIGHT);
    const std::string text = StringTools::strip(OCR::ocr_read(Language::English, scaled));
    const char* end = text.data() + text.size();
    size_t box = 0;
    const auto [slash, box_error] = std::from_chars(text.data(), end, box);
    if (box_error != std::errc{} || slash == end || *slash != '/'){
        return std::nullopt;
    }

    size_t capacity = 0;
    const auto [capacity_end, capacity_error] = std::from_chars(slash + 1, end, capacity);
    if (capacity_error != std::errc{} || capacity_end != end || box == 0 || box > capacity || capacity > 300){
        return std::nullopt;
    }
    return box;
}

class Test_BoxNumberReader : public ImageUnitTest{
public:
    Test_BoxNumberReader(const std::string& image, std::optional<size_t> expected)
        : ImageUnitTest("PokemonHome::BoxNumberReader - " + image, {image})
        , m_expected(expected)
    {
        m_memory += PADDLE_OCR_SINGLE_LINE_MEMORY_ESTIMATE;
    }

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        const auto result = BoxNumberReader().read_box_number(images[0]);
        if (result == m_expected){
            return true;
        }
        return "Expected box " + (m_expected ? std::to_string(*m_expected) : "none")
            + ", read " + (result ? std::to_string(*result) : "none");
    }

private:
    std::optional<size_t> m_expected;
};

void add_tests_BoxNumberReader(UnitTestDatabase& database){
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/BoxView-1.png", 21);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_01.png", 2);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_02.png", 2);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_03.png", 2);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_04.png", 2);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_05.png", 2);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_06.png", 2);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_07.png", 3);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_08.png", 1);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_BoxSpaces_09.png", 4);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row0_Column0_01.png", 219);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row0_Column0_02.png", 219);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row2_Column0_01.png", 299);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row3_Column3_01.png", 219);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Multi_Grid_Row3_Column3_02.png", 219);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxName_01.png", 300);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxName_02.png", 300);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_01.png", 4);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_02.png", 4);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_03.png", 4);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_04.png", 3);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_05.png", 3);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_06.png", 8);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_07.png", 8);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_08.png", 8);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_09.png", 10);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_10.png", 10);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_BoxSpaces_11.png", 300);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row0_Column0_01.png", 21);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row0_Column0_02.png", 146);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_Grid_Row3_Column0_01.png", 300);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_Newest30_01.png", std::nullopt);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_Newest30_02.png", std::nullopt);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Normal_Newest30_03.png", std::nullopt);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_01.png", 103);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_02.png", 103);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_03.png", 103);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_04.png", 168);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxName_05.png", 168);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_01.png", 15);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_02.png", 15);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_03.png", 15);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_04.png", 24);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_05.png", 24);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_06.png", 28);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_07.png", 28);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_08.png", 87);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_09.png", 87);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_10.png", 87);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_11.png", 100);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_12.png", 101);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_13.png", 101);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_14.png", 101);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_15.png", 103);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_BoxSpaces_16.png", 103);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column0_01.png", 168);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column0_02.png", 168);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column5_01.png", 182);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_Grid_Row2_Column5_02.png", 182);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_Newest30_01.png", std::nullopt);
    database.add<Test_BoxNumberReader>("PokemonHome/BoxView/SelectionArrow_Relocate_Newest30_02.png", std::nullopt);
    database.add<Test_BoxNumberReader>("PokemonHome/SummaryScreen/annihilape_Regular.png", std::nullopt);
}

}
}
}
