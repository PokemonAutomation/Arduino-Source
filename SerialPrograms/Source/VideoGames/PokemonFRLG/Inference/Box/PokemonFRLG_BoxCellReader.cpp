/*  Box Cell Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cctype>
#include <set>
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "VideoGames/PokemonFRLG/Inference/PokemonFRLG_OcrPreprocessing.h"
#include "PokemonFRLG_BoxCellOccupiedDetector.h"
#include "PokemonFRLG_BoxCellReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

namespace{

const Pokemon::PokemonNameReader& gen3_name_reader(){
    static const Pokemon::PokemonNameReader reader([]{
        std::set<std::string> species;
        constexpr size_t GEN3_LAST_NATIONAL_DEX = 386;
        const auto& national_dex = Pokemon::NATIONAL_DEX_SLUGS();
        for (size_t index = 0; index < GEN3_LAST_NATIONAL_DEX; index++){
            species.insert(national_dex[index]);
        }
        return species;
    }());
    return reader;
}

int parse_level(const std::string& text){
    // Strip the Lv label before finding digits; OCR can mistake its L for 1.
    const size_t label_end = text.find_last_of("vV");
    const size_t start = text.find_first_of("0123456789", label_end == std::string::npos ? 0 : label_end + 1);
    if (start == std::string::npos){
        return -1;
    }
    int level = 0;
    for (size_t i = start; i < text.size(); i++){
        const unsigned char ch = (unsigned char)text[i];
        if (std::isspace(ch)){
            continue;
        }
        if (ch < '0' || ch > '9'){
            return -1;
        }
        level = level * 10 + ch - '0';
        if (level > 100){
            return -1;
        }
    }
    return level >= 1 ? level : -1;
}

}

BoxCellReader::BoxCellReader(Color color)
    : m_color(color)
    , m_name_box(0.0006, 0.628, 0.32, 0.099)
    , m_level_box(0.0697, 0.731, 0.189, 0.0763)
{}

void BoxCellReader::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& game_box = GameSettings::instance().GAME_BOX;
    items.add(m_color, game_box.inner_to_outer(m_name_box));
    items.add(m_color, game_box.inner_to_outer(m_level_box));
}

std::string BoxCellReader::read_name(Logger& logger, Language language, const ImageViewRGB32& screen) const{
    if (!BoxCellOccupiedDetector().detect(screen)){
        return "";
    }
    const ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    const ImageViewRGB32 name_region = extract_box_reference(game_screen, m_name_box);
    // Preserve letter details first: blur can turn D into O and yield a confident
    // wrong species (e.g. DODUO -> Sudowoodo). Smooth only if the sharp read fails.
    for (int blur_kernel_size : {0, 3}){
        const ImageRGB32 name_ready = preprocess_for_ocr(name_region, blur_kernel_size, 1);
        const OCR::StringMatchResult result = gen3_name_reader().read_substring(
            logger, language, name_ready, BRIGHT_TEXT_FILTERS()
        );
        if (result.results.size() == 1){
            return result.results.begin()->second.token;
        }
        std::set<std::string> candidates;
        for (const auto& candidate : result.results){
            candidates.insert(candidate.second.token);
        }
        if (candidates == std::set<std::string>{"nidoran-f", "nidoran-m"}){
            const std::string name = read_nidoran_symbol(name_ready, BRIGHT_TEXT_FILTERS());
            if (!name.empty()){
                return name;
            }
        }
    }
    logger.log("Failed to read box species name unambiguously.", COLOR_RED);
    return "";
}

int BoxCellReader::read_level(Logger& logger, const ImageViewRGB32& screen) const{
    if (!BoxCellOccupiedDetector().detect(screen)){
        return -1;
    }
    const ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    const ImageRGB32 level_ready = preprocess_for_ocr(extract_box_reference(game_screen, m_level_box), 3, 1);
    for (const auto& filter : BRIGHT_TEXT_FILTERS()){
        size_t text_pixels = 0;
        ImageRGB32 filtered = to_blackwhite_rgb32_range(text_pixels, level_ready, true, filter.mins, filter.maxs);
        if (text_pixels == 0){
            continue;
        }
        const int level = parse_level(OCR::ocr_read(Language::English, filtered, OCR::PageSegMode::SINGLE_LINE));
        if (level != -1){
            return level;
        }
    }
    logger.log("Failed to read box level.", COLOR_RED);
    return -1;
}

class Test_BoxCellReader_Name : public UnitTest{
public:
    Test_BoxCellReader_Name(const std::string& image, const std::string& expected)
        : UnitTest("PokemonFRLG::BoxCellReader_Name - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        const std::string result = BoxCellReader().read_name(logger, Language::English, image);
        if (result == m_expected){
            return true;
        }
        return "Expected species: \"" + m_expected + "\", received: \"" + result + "\".";
    }

private:
    std::string m_image;
    std::string m_expected;
};

class Test_BoxCellReader_Level : public UnitTest{
public:
    Test_BoxCellReader_Level(const std::string& image, int expected)
        : UnitTest("PokemonFRLG::BoxCellReader_Level - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        const int result = BoxCellReader().read_level(logger, image);
        if (result == m_expected){
            return true;
        }
        return "Expected level: " + std::to_string(m_expected) + ", received: " + std::to_string(result);
    }

private:
    std::string m_image;
    int m_expected;
};

void add_tests_BoxCellReader(UnitTestDatabase& database){
    // Name tests
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/Arcanine_M_100_White.png", "arcanine");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/BoxHand_Closed_Gold.png", "dodrio");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/BoxHand_Closed_White.png", "exeggcute");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/BoxHand_Gold_HD60Pro_NV12.png", "farfetch-d");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/BoxHand_Gold_HD60Pro_YUYV.png", "farfetch-d");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/BoxHand_White_HD60Pro_NV12.png", "farfetch-d");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/BoxHand_White_HD60Pro_YUYV.png", "farfetch-d");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/BoxSelection_Gold.png", "");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/BoxSelection_Gold2.png", "raichu");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/BoxSelection_White.png", "");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/BoxSelection_White2.png", "raichu");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/Caterpie_F_4_White.png", "caterpie");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/CloseSelection_Gold.png", "");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/CloseSelection_White.png", "");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/Diglett_F_20_White.png", "diglett");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/Doduo-NV12.png", "doduo");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/Doduo-YUYV.png", "doduo");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/Dragonite_F_100_Whtie.png", "dragonite");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/Nidokinq_M_23_White.png", "nidoking");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/nidoran-f-mjpeg.png", "nidoran-f");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/nidoran-f-NV12.png", "nidoran-f");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/nidoran-f-YUYV.png", "nidoran-f");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/nidoran-m-mjpeg.png", "nidoran-m");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/nidoran-m-NV12.png", "nidoran-m");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/nidoran-m-YUYV.png", "nidoran-m");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/PartySelection_Gold.png", "");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/PartySelection_White.png", "");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/Rhydon-NV12.png", "rhydon");
    database.add<Test_BoxCellReader_Name>("PokemonFRLG/Box/Rhydon-YUYV.png", "rhydon");

    // Level tests
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/Arcanine_M_100_White.png", 100);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/BoxHand_Closed_Gold.png", 31);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/BoxHand_Closed_White.png", 25);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/BoxHand_Gold_HD60Pro_NV12.png", 33);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/BoxHand_Gold_HD60Pro_YUYV.png", 33);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/BoxHand_White_HD60Pro_NV12.png", 33);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/BoxHand_White_HD60Pro_YUYV.png", 33);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/BoxSelection_Gold.png", -1);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/BoxSelection_Gold2.png", 100);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/BoxSelection_White.png", -1);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/BoxSelection_White2.png", 100);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/Caterpie_F_4_White.png", 4);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/CloseSelection_Gold.png", -1);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/CloseSelection_White.png", -1);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/Diglett_F_20_White.png", 20);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/Doduo-NV12.png", 26);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/Doduo-YUYV.png", 26);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/Dragonite_F_100_Whtie.png", 100);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/Nidokinq_M_23_White.png", 23);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/nidoran-f-mjpeg.png", 6);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/nidoran-f-NV12.png", 6);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/nidoran-f-YUYV.png", 6);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/nidoran-m-mjpeg.png", 6);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/nidoran-m-NV12.png", 6);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/nidoran-m-YUYV.png", 6);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/PartySelection_Gold.png", -1);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/PartySelection_White.png", -1);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/Rhydon-NV12.png", 42);
    database.add<Test_BoxCellReader_Level>("PokemonFRLG/Box/Rhydon-YUYV.png", 42);
}

}
}
}
