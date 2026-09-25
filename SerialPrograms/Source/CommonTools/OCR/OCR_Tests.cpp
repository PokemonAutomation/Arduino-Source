/*  OCR Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
// #include "Common/Cpp/Strings/Unicode.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "OCR_Routines.h"
#include "OCR_StringNormalization.h"
#include "OCR_Tests.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace OCR{



void add_tests(UnitTestDatabase& database){
    add_tests_raw_OCR(database);
    add_tests_number_waterfill_OCR(database);
    add_tests_multiline_OCR(database);
}

class Test_RawOCR : public UnitTest{
public:
    Test_RawOCR(
        const std::string& image,
        Language language,
        const std::string& expected
    )
        : UnitTest("OCR::RawOCR - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_language(language)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);

        std::string result = ocr_read(m_language, image);
        logger.log("Raw OCR read: " + result);

        return normalize_utf32(result) == normalize_utf32(m_expected);
    };

private:
    std::string m_image;
    Language m_language;
    std::string m_expected;
};

class Test_NumberWaterfillOCR : public UnitTest{
public:
    Test_NumberWaterfillOCR(
        const std::string& image_path,
        int expected,
        uint32_t rgb32_min, uint32_t rgb32_max,
        bool text_inside_range
    )
        : UnitTest("OCR::NumberWaterfillOCR - " + image_path)
        , m_image_path(UNIT_TEST_RESOURCE_PATH() + image_path)
        , m_expected(expected)
        , m_rgb32_min(rgb32_min)
        , m_rgb32_max(rgb32_max)
        , m_text_inside_range(text_inside_range)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image_path);

        int result = OCR::read_number_waterfill(
            logger, image,
            m_rgb32_min, m_rgb32_max, m_text_inside_range
        );
        // logger.log("Raw OCR read: " + std::to_string(result));

        return result == m_expected;
    };

private:
    std::string m_image_path;
    int m_expected;
    uint32_t m_rgb32_min;
    uint32_t m_rgb32_max;
    bool m_text_inside_range;
};

void add_tests_raw_OCR(UnitTestDatabase& database){
    database.add<Test_RawOCR>("OCR/letter-i-tall-1.jpg", Language::English, "I");
    database.add<Test_RawOCR>("OCR/letter-i-tall-2.jpg", Language::English, "I");
    database.add<Test_RawOCR>("OCR/letter-i-wide-1.jpg", Language::English, "I");
    database.add<Test_RawOCR>("OCR/letter-i-wide-2.jpg", Language::English, "I");
    database.add<Test_RawOCR>("OCR/sentence-1-1.jpg", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-1-wide.jpg", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-1-tall.jpg", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-2.jpg", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-2-wide.jpg", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-2-tall.jpg", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-3.jpg", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-3-wide.jpg", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-3-tall.jpg", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/german-nature-sanft.png", Language::German, "Wesen: SANFT");
    database.add<Test_RawOCR>("OCR/Lv.1-0.png", Language::English, "Lv. 1");
    database.add<Test_RawOCR>("OCR/Lv.1-1.png", Language::English, "Lv. 1");
    database.add<Test_RawOCR>("OCR/Lv.1-2.png", Language::English, "Lv. 1");
    database.add<Test_RawOCR>("OCR/Lv.1-3.png", Language::English, "Lv. 1");
    database.add<Test_RawOCR>("OCR/Lv.1-4.png", Language::English, "Lv. 1");
    database.add<Test_RawOCR>("OCR/Lv.1-5.png", Language::English, "Lv. 1");
    database.add<Test_RawOCR>("OCR/dash-1.png", Language::English, "---");
    database.add<Test_RawOCR>("OCR/dash-2.png", Language::English, "---");
    database.add<Test_RawOCR>("OCR/dash-3.png", Language::English, "---");
    database.add<Test_RawOCR>("OCR/num-1.png", Language::English, "1");
}


void add_tests_number_waterfill_OCR(UnitTestDatabase& database){
    database.add<Test_NumberWaterfillOCR>("OCR/date-1.png", 11, 0xff000000, 0xffff7fff, true);
}



// Crop `box` out of a screenshot, run multi-line OCR on it, and check that it finds
// exactly the expected lines in order. Each line is compared after
// `normalize_utf32()`, which ignores punctuation and whitespace.
class Test_MultilineOCR : public UnitTest{
public:
    Test_MultilineOCR(
        const std::string& image,
        const ImagePixelBox& box,
        Language language,
        std::vector<std::string> expected_lines
    )
        : UnitTest("OCR::MultilineOCR - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_box(box)
        , m_language(language)
        , m_expected_lines(std::move(expected_lines))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        ImageViewRGB32 cropped = extract_box_reference(image, m_box);

        std::string result = ocr_read(m_language, cropped, PageSegMode::SINGLE_BLOCK);
        logger.log("Multi-line OCR read: " + result);

        std::vector<std::string> lines;
        size_t start = 0;
        while (true){
            size_t end = result.find('\n', start);
            lines.emplace_back(result.substr(start, end - start));
            if (end == std::string::npos){
                break;
            }
            start = end + 1;
        }

        if (lines.size() != m_expected_lines.size()){
            logger.log(
                "Expected " + std::to_string(m_expected_lines.size()) +
                " lines, got " + std::to_string(lines.size())
            );
            return false;
        }
        for (size_t c = 0; c < lines.size(); c++){
            if (normalize_utf32(lines[c]) != normalize_utf32(m_expected_lines[c])){
                logger.log("Line " + std::to_string(c) + " mismatch. Expected: " + m_expected_lines[c]);
                return false;
            }
        }
        return true;
    };

private:
    std::string m_image;
    ImagePixelBox m_box;
    Language m_language;
    std::vector<std::string> m_expected_lines;
};

void add_tests_multiline_OCR(UnitTestDatabase& database){
    //  Already cropped to a single line, but with lots of empty space around it.
    database.add<Test_MultilineOCR>(
        "OCR/sentence-1-1-tall.jpg", ImagePixelBox(0, 0, 564, 500), Language::English,
        std::vector<std::string>{"You hurry to the Pokemon Center, shielding your"}
    );
    database.add<Test_MultilineOCR>(
        "OCR/sentence-1-1-wide.jpg", ImagePixelBox(0, 0, 1000, 41), Language::English,
        std::vector<std::string>{"You hurry to the Pokemon Center, shielding your"}
    );

    //  Two-line FRLG dialog boxes: blue text on white.
    database.add<Test_MultilineOCR>(
        "PokemonFRLG/AdvanceWhiteDialogDetector/English-Bulba_True.png",
        ImagePixelBox(270, 790, 1650, 1000), Language::English,
        std::vector<std::string>{"I see! BULBASAUR is your choice.", "It's very easy to raise."}
    );
    database.add<Test_MultilineOCR>(
        "PokemonFRLG/AdvanceWhiteDialogDetector/Ger-KennyExile-Char_True.png",
        ImagePixelBox(270, 790, 1650, 1000), Language::German,
        std::vector<std::string>{"Ah! GLUMANDA soll es sein!", "Trainiere es geduldig!"}
    );

    //  SV dialog box: speaker name (white on dark blue) above the dialog text (dark on white).
    database.add<Test_MultilineOCR>(
        "PokemonSV/DialogBoxDetector/French_True.png",
        ImagePixelBox(430, 740, 1470, 975), Language::French,
        std::vector<std::string>{"Clavel", "Poussacha, le Pokémon Chat Plante."}
    );
}



}
}
