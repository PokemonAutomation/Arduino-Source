/*  Wild Encounter Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <opencv2/imgproc.hpp>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "Tests/ImageUnitTest.h"
#include "Tests/TestUtils.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_WildEncounterReader.h"
#include "PokemonFRLG_OcrPreprocessing.h"
#include "PokemonFRLG_DigitReader.h"
namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {


WildEncounterReader::WildEncounterReader(Color color)
    : m_box_name(0.075, 0.120, 0.260, 0.063) 
    , m_box_name_jpn(0.075, 0.120, 0.232, 0.063)
    // , m_box_level(0.325, 0.120, 0.092, 0.063)
    {}

void WildEncounterReader::make_overlays(VideoOverlaySet& items) const {
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_name));
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_name_jpn));
    // items.add(m_color, GAME_BOX.inner_to_outer(m_box_level));
}

PokemonFRLG_WildEncounter WildEncounterReader::read_encounter(
    Logger& logger, Language language,
    const ImageViewRGB32& frame, 
    const std::set<std::string>& subset,
    double max_log10p
){
    const bool jpn = language == Language::Japanese;

    PokemonFRLG_WildEncounter encounter; 
    ImageViewRGB32 game_screen =
            extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 name_box = extract_box_reference(game_screen, jpn ? m_box_name_jpn : m_box_name);

    //  Dark text on the light battle HP bar. Blur to close the gaps in the GBA
    //  font, then let the matcher try each threshold in turn.
    ImageRGB32 name_ready = preprocess_for_ocr(name_box);

    OCR::StringMatchResult name_result;
    if (subset.size() == 0){
        name_result = Pokemon::PokemonNameReader::instance().read_substring(
            logger, language, name_ready,
            DARK_TEXT_FILTERS(),
            0.01, 0.50, max_log10p);
    }else{
        name_result = Pokemon::PokemonNameReader(subset).read_substring(
            logger, language, name_ready,
            DARK_TEXT_FILTERS(),
            0.01, 0.50, max_log10p);
    }
   
    if (!name_result.results.empty()){
        encounter.name = name_result.results.begin()->second.token;
    }else{
    logger.log("Failed to read species name.", COLOR_RED);
        if (GlobalSettings::instance().SAVE_DEBUG_IMAGES){
            name_box.save("DebugDumps/ocr_encounter_box.png");
            name_ready.save("DebugDumps/ocr_encounter_ready.png");
            game_screen.save("DebugDumps/ocr_encounter_frame.png");
        }
    }
    return encounter;
}


class Test_WildEncounterReader : public ImageUnitTest{
public:
    Test_WildEncounterReader(const std::string& image)
        : ImageUnitTest("PokemonFRLG::WildEncounterReader - " + image, {image})
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        const std::vector<std::string> words =
                parse_words(Filesystem::Path(image_paths()[0]).stem().string());
        if (words.size() < 2){
            return "Error: filename must be <anything>_<language>_<species>.";
        }

        std::string language_word = words[words.size() - 2];
        const std::string& target_species = words[words.size() - 1];

        Language language = language_code_to_enum(language_word);
        if (language == Language::None || language == Language::EndOfList){
            return "Error: invalid language word in filename: " + language_word;
        }

        const ImageRGB32& image = images[0];
        WildEncounterReader reader;
        PokemonFRLG_WildEncounter encounter =
                reader.read_encounter(logger, language, image, {});

        TEST_RESULT_COMPONENT_EQUAL_STR(encounter.name, target_species, "species");
        return true;
    };
};


void add_tests_WildEncounterReader(UnitTestDatabase& database){
    database.add<Test_WildEncounterReader>("PokemonFRLG/WildEncounterReader/eng_chansey.jpg");
    database.add<Test_WildEncounterReader>("PokemonFRLG/WildEncounterReader/eng_machop.jpg");
    database.add<Test_WildEncounterReader>("PokemonFRLG/WildEncounterReader/eng_kakuna.jpg");
    database.add<Test_WildEncounterReader>("PokemonFRLG/WildEncounterReader/eng_ho-oh.jpg");
    database.add<Test_WildEncounterReader>("PokemonFRLG/WildEncounterReader/eng_heracross.jpg");
    database.add<Test_WildEncounterReader>("PokemonFRLG/WildEncounterReader/eng_dragonair.jpg");
    database.add<Test_WildEncounterReader>("PokemonFRLG/WildEncounterReader/eng_articuno.jpg");
    database.add<Test_WildEncounterReader>("PokemonFRLG/WildEncounterReader/fra_snorlax.jpg");
    database.add<Test_WildEncounterReader>("PokemonFRLG/WildEncounterReader/dark_eng_chansey.png");
}


} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

