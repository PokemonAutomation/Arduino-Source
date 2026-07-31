/*  OCR Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
// #include "Common/Cpp/Strings/Unicode.h"
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

void add_tests_raw_OCR(UnitTestDatabase& database){
    database.add<Test_RawOCR>("OCR/letter-i-tall-1", Language::English, "I");
    database.add<Test_RawOCR>("OCR/letter-i-tall-2", Language::English, "I");
    database.add<Test_RawOCR>("OCR/letter-i-wide-1", Language::English, "I");
    database.add<Test_RawOCR>("OCR/letter-i-wide-2", Language::English, "I");
    database.add<Test_RawOCR>("OCR/sentence-1-1", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-1-wide", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-1-tall", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-2", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-2-wide", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-2-tall", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-3", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-3-wide", Language::English, "You hurry to the Pokemon Center, shielding your");
    database.add<Test_RawOCR>("OCR/sentence-1-3-tall", Language::English, "You hurry to the Pokemon Center, shielding your");        
}



}
}
