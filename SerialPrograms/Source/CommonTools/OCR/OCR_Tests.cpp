/*  OCR Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Common/Cpp/Strings/Unicode.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "OCR_Routines.h"
#include "OCR_StringNormalization.h"
#include "OCR_TextMatcher.h"
#include "OCR_Tests.h"

#include <cmath>
#include <random>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace OCR{



void add_tests(UnitTestDatabase& database){
    add_tests_text_matcher(database);
    add_tests_raw_OCR(database);
    add_tests_number_waterfill_OCR(database);
    Pokemon::add_tests_PokemonNameReader(database);
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
    // database.add<Test_RawOCR>("OCR/dash-2.png", Language::English, "---");
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



// ============================================================================
//  Weighted Levenshtein distance tests (pure logic, no images).
// ============================================================================

namespace {

//  Substitution cost of 1.0 for every distinct pair: reproduces the
//  unweighted Levenshtein distance.
double unit_substitution_cost(char32_t ch0, char32_t ch1){
    (void)ch0;
    (void)ch1;
    return 1.0;
}

//  A cheap "looks-alike" pair: k -> s costs 0.25, everything else 1.0.
double kitten_substitution_cost(char32_t ch0, char32_t ch1){
    if (ch0 == U'k' && ch1 == U's'){
        return 0.25;
    }
    return 1.0;
}

int g_counting_sub_calls = 0;
int g_counting_equal_calls = 0;
double counting_substitution_cost(char32_t ch0, char32_t ch1){
    if (ch0 == ch1){
        g_counting_equal_calls++;
    }
    g_counting_sub_calls++;
    return 42.0;
}

//  Fuzz machinery: a random substitution-cost table over a small alphabet,
//  consulted through a plain function pointer.
const char32_t FUZZ_ALPHABET[] = {'a', 'b', 'c', 'k', '0', u'霹'};
const size_t FUZZ_ALPHABET_SIZE = sizeof(FUZZ_ALPHABET) / sizeof(FUZZ_ALPHABET[0]);
double g_fuzz_sub_cost[FUZZ_ALPHABET_SIZE][FUZZ_ALPHABET_SIZE];

size_t fuzz_alpha_index(char32_t ch){
    for (size_t i = 0; i < FUZZ_ALPHABET_SIZE; i++){
        if (FUZZ_ALPHABET[i] == ch){
            return i;
        }
    }
    return 0;
}

double fuzz_substitution_cost(char32_t ch0, char32_t ch1){
    return g_fuzz_sub_cost[fuzz_alpha_index(ch0)][fuzz_alpha_index(ch1)];
}

//  Independent reference implementation: tries every possible matched region
//  [s, e) of input. The region must align input[s..e) fully with
//  match_target (1.0 per in-region insertion/deletion), and the context on
//  both sides costs prepostfix_cost per character.
double brute_force_weighted_levenshtein(
    double prepostfix_cost,
    const std::u32string& match_target, const std::u32string& input
){
    const size_t target_len = match_target.size();
    const size_t input_len = input.size();
    const double context_cost = std::min(prepostfix_cost, 1.0);

    double best = (double)target_len + (double)input_len * context_cost;    //  Empty region.

    for (size_t s = 0; s <= input_len; s++){
        for (size_t e = s; e <= input_len; e++){
            const size_t len = e - s;

            //  cost[i][j] = cost of aligning input[s+i..e) with
            //  match_target[j..target_len).
            std::vector<std::vector<double>> cost(len + 1, std::vector<double>(target_len + 1, 0.0));
            for (size_t i = 0; i <= len; i++){
                cost[i][target_len] = (double)(len - i);    //  Delete the rest of the region.
            }
            for (size_t j = 0; j <= target_len; j++){
                cost[len][j] = (double)(target_len - j);    //  Insert the rest of match_target.
            }
            for (size_t i = len; i-- > 0;){
                const char32_t ch0 = input[s + i];
                for (size_t j = target_len; j-- > 0;){
                    const char32_t ch1 = match_target[j];
                    double sub = cost[i + 1][j + 1];
                    if (ch0 != ch1){
                        sub += g_fuzz_sub_cost[fuzz_alpha_index(ch0)][fuzz_alpha_index(ch1)];
                    }
                    cost[i][j] = std::min(
                        sub,
                        std::min(cost[i + 1][j] + 1.0, cost[i][j + 1] + 1.0)
                    );
                }
            }

            best = std::min(
                best, (double)(s + input_len - e) * context_cost + cost[0][0]
            );
        }
    }

    return best;
}



} //  namespace



class Test_WeightedLevenshtein : public UnitTest{
public:
    Test_WeightedLevenshtein()
        : UnitTest("OCR::TextMatcher - WeightedLevenshtein")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        (void)scope;

        const std::vector<std::pair<std::u32string, std::u32string>> pairs{
            {std::u32string(), std::u32string()},
            {U"abc", U""},
            {U"", U"abc"},
            {U"kitten", U"sitting"},
            {U"abc", U"abc"},
            {U"a", U"ab"},
            {U"axb", U"ab"},
            {U"abcde", U"ace"},
            {U"the cat ran", U"cat"},
            {utf8_to_utf32("霹雳火"), utf8_to_utf32("霹雳水")},
        };

        //  1. Unit substitution cost: free context must reproduce the
        //     substring variant, and unit context the full-string variant.
        for (size_t i = 0; i < pairs.size(); i++){
            const std::u32string& input = pairs[i].first;
            const std::u32string& match_target = pairs[i].second;

            double free_context = levenshtein_distance(
                0.0, unit_substitution_cost, match_target, input
            );
            double free_context_expected = (double)levenshtein_distance_substring(
                match_target, input
            );
            if (free_context != free_context_expected){
                logger.log(
                    "Free-context mismatch: input = " + utf32_to_str(input) +
                    ", match_target = " + utf32_to_str(match_target) +
                    " (got " + std::to_string(free_context) + ", expected " +
                    std::to_string(free_context_expected) + ")"
                );
                return false;
            }

            double full_context = levenshtein_distance(
                1.0, unit_substitution_cost, match_target, input
            );
            double full_context_expected = (double)levenshtein_distance(input, match_target);
            if (full_context != full_context_expected){
                logger.log(
                    "Full-context mismatch: input = " + utf32_to_str(input) +
                    ", match_target = " + utf32_to_str(match_target) +
                    " (got " + std::to_string(full_context) + ", expected " +
                    std::to_string(full_context_expected) + ")"
                );
                return false;
            }
        }

        //  2. A cheap "looks-alike" substitution: only k -> s differs.
        if (levenshtein_distance(1.0, kitten_substitution_cost, U"sitten", U"kitten") != 0.25){
            return UnitTestResult("Expected 0.25 for kitten -> sitten with k -> s costing 0.25.");
        }

        //  3. Context costs: 4 context characters around "cat" in "xxcatxx".
        if (levenshtein_distance(0.0, unit_substitution_cost, U"cat", U"xxcatxx") != 0.0
            || levenshtein_distance(0.25, unit_substitution_cost, U"cat", U"xxcatxx") != 1.0
            || levenshtein_distance(1.0, unit_substitution_cost, U"cat", U"xxcatxx") != 4.0){
            return UnitTestResult("Context costs for xxcatxx / cat were not 0 / 1.0 / 4.0.");
        }

        //  4. Context costs above 1.0 behave like 1.0 (skipping a character
        //     is never more expensive than deleting it in the region).
        if (levenshtein_distance(5.0, unit_substitution_cost, U"cat", U"xxcatxx") != 4.0){
            return UnitTestResult("Context cost 5.0 should behave like 1.0.");
        }

        //  5. Empty strings.
        if (levenshtein_distance(0.0, unit_substitution_cost, U"abc", U"") != 3.0
            || levenshtein_distance(0.5, unit_substitution_cost, U"abc", U"") != 3.0){
            return UnitTestResult("Empty input must cost len(match_target) insertions.");
        }
        if (levenshtein_distance(0.0, unit_substitution_cost, U"", U"abc") != 0.0
            || levenshtein_distance(0.5, unit_substitution_cost, U"", U"abc") != 1.5
            || levenshtein_distance(2.0, unit_substitution_cost, U"", U"abc") != 3.0){
            return UnitTestResult("Empty match_target must cost len(input) context.");
        }

        //  6. Identical characters cost 0 without calling the cost function
        //     for them (distinct pairs may still be evaluated).
        g_counting_sub_calls = 0;
        g_counting_equal_calls = 0;
        if (levenshtein_distance(0.0, counting_substitution_cost, U"abc", U"abc") != 0.0
            || g_counting_equal_calls != 0){
            return UnitTestResult("The cost function must never be called with identical characters.");
        }

        //  7. A substitution cost above insert + delete decomposes into
        //     delete + insert. In the substring variant the trailing "c" of
        //     "abc" -> "abd" is free context (1.0); in the full-string
        //     variant it costs 2.0, not the 42.0 substitution.
        g_counting_sub_calls = 0;
        g_counting_equal_calls = 0;
        if (levenshtein_distance(0.0, counting_substitution_cost, U"abd", U"abc") != 1.0
            || g_counting_equal_calls != 0){
            return UnitTestResult("Substring variant of abc -> abd should cost 1.0 (free suffix).");
        }
        g_counting_sub_calls = 0;
        g_counting_equal_calls = 0;
        if (levenshtein_distance(1.0, counting_substitution_cost, U"abd", U"abc") != 2.0
            || g_counting_equal_calls != 0){
            return UnitTestResult("Full-string variant of abc -> abd should decompose into delete + insert.");
        }

        return true;
    };

};



class Test_WeightedLevenshtein_Fuzz : public UnitTest{
public:
    Test_WeightedLevenshtein_Fuzz()
        : UnitTest("OCR::TextMatcher - WeightedLevenshteinFuzz")
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        (void)scope;

        const double COSTS[] = {0.0, 0.25, 0.5, 0.75, 1.0, 1.5};
        const double CONTEXTS[] = {0.0, 0.3, 1.0, 2.5};

        std::mt19937 rng(0xC0FFEE42u);
        std::uniform_int_distribution<size_t> alpha_index(0, FUZZ_ALPHABET_SIZE - 1);
        std::uniform_int_distribution<size_t> string_length(0, 5);
        std::uniform_int_distribution<size_t> cost_index(0, sizeof(COSTS) / sizeof(COSTS[0]) - 1);
        std::uniform_int_distribution<size_t> context_index(0, sizeof(CONTEXTS) / sizeof(CONTEXTS[0]) - 1);

        for (int trial = 0; trial < 200; trial++){
            //  Fresh random cost table.
            for (auto& row : g_fuzz_sub_cost){
                for (auto& entry : row){
                    entry = COSTS[cost_index(rng)];
                }
            }

            std::u32string match_target, input;
            for (size_t i = 0; i < string_length(rng); i++){
                match_target += FUZZ_ALPHABET[alpha_index(rng)];
            }
            for (size_t i = 0; i < string_length(rng); i++){
                input += FUZZ_ALPHABET[alpha_index(rng)];
            }
            const double context = CONTEXTS[context_index(rng)];

            const double expected = brute_force_weighted_levenshtein(
                context, match_target, input
            );
            const double actual = levenshtein_distance(
                context, fuzz_substitution_cost, match_target, input
            );
            const double tolerance = 1e-9 * std::max(1.0, std::max(expected, actual));
            if (std::abs(actual - expected) > tolerance){
                logger.log(
                    "Fuzz mismatch (trial " + std::to_string(trial) + ", context = " +
                    std::to_string(context) + "): match_target = " +
                    utf32_to_str(match_target) + ", input = " + utf32_to_str(input) +
                    ", expected = " + std::to_string(expected) +
                    ", got = " + std::to_string(actual)
                );
                return false;
            }
        }

        return true;
    };

};



void add_tests_text_matcher(UnitTestDatabase& database){
    database.add<Test_WeightedLevenshtein>();
    database.add<Test_WeightedLevenshtein_Fuzz>();
}



}
}
