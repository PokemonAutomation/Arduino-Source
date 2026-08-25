/*  Trainer ID Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonFRLG_TrainerIdReader.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/ImageManip.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG/PokemonFRLG_Tests.h"
#include "Tests/TestUtils.h"
#include "PokemonFRLG_DigitReader.h"
#include <opencv2/imgproc.hpp>
#include <sstream>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {

TrainerIdReader::TrainerIdReader(Color color)
    : m_color(color)
    , m_box_tid(0.742683, 0.117314, 0.129734, 0.076006)
    , m_box_tid_jpn(0.712981, 0.118836, 0.207212, 0.077373)
{}

void TrainerIdReader::make_overlays(VideoOverlaySet &items) const {
    const BoxOption &GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_tid));
}

uint16_t TrainerIdReader::read_tid(
    Logger& logger, Language language, const ImageViewRGB32& frame
){
    ImageViewRGB32 game_screen =
            extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    
    ImageViewRGB32 tid_region = extract_box_reference(game_screen, language == Language::Japanese ? m_box_tid_jpn : m_box_tid);

    // waterfill segmentation + template matching
    // against the PokemonFRLG/Digits/0-9.png templates.
    return uint16_t(read_digits_waterfill_template(logger, tid_region, DigitTemplateType::DialogBox));
}


class Test_TrainerIdReader : public UnitTest{
public:
    Test_TrainerIdReader(const std::string& image)
        : UnitTest("PokemonFRLG::TrainerIdReader - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        const std::vector<std::string> words =
                parse_words(Filesystem::Path(m_image).stem().string());
        if (words.size() < 2){
            return "Error: filename must be <anything>_<language code>_<trainer id>.";
        }

        const std::string& language_word = words[words.size() - 2];
        Language language = language_code_to_enum(language_word);
        if (language == Language::None || language == Language::EndOfList){
            return "Error: invalid language word in filename: " + language_word;
        }
        int target_tid = 0;
        if (!parse_int(words.back(), target_tid)){
            return "Error: filename must end with the trainer ID: " + words.back();
        }

        ImageRGB32 image(m_image);
        TrainerIdReader reader;
        int tid = reader.read_tid(logger, language, image);

        TEST_RESULT_COMPONENT_EQUAL_STR(tid, target_tid, "trainer id");
        return true;
    };

private:
    std::string m_image;
};


void add_tests_TrainerIdReader(UnitTestDatabase& database){
    database.add<Test_TrainerIdReader>("PokemonFRLG/TrainerIdReader/tom_eng_60895.jpg");
    database.add<Test_TrainerIdReader>("PokemonFRLG/TrainerIdReader/nyash_jpn_45345.png");
    database.add<Test_TrainerIdReader>("PokemonFRLG/TrainerIdReader/alberto_spa_65385.png");
}


} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

