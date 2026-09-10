/*  Pokemon Home Test Database Generator
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
#include "Pokemon/Pokemon_OriginMarks.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_Types.h"
#include "PokemonHome/Inference/PokemonHome_BallReader.h"
#include "PokemonHome/Inference/PokemonHome_AlphaDetector.h"
#include "PokemonHome/Inference/PokemonHome_BoxGenderDetector.h"
#include "PokemonHome/Inference/PokemonHome_BoxViewDetector.h"
#include "PokemonHome/Inference/PokemonHome_ButtonDetector.h"
#include "PokemonHome/Inference/PokemonHome_GigantamaxDetector.h"
#include "PokemonHome/Inference/PokemonHome_OriginMarkReader.h"
#include "PokemonHome/Inference/PokemonHome_SelectionArrowDetector.h"
#include "PokemonHome/Inference/PokemonHome_ShinyDetector.h"
#include "PokemonHome/Inference/PokemonHome_SummaryReader.h"
#include "PokemonHome/Inference/PokemonHome_SummaryScreenDetector.h"
#include "PokemonHome/Inference/PokemonHome_TeraTypeReader.h"
#include "PokemonHome_TestDatabaseGenerator.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
using namespace Pokemon;


TestDatabaseGenerator_Descriptor::TestDatabaseGenerator_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonHome:TestDatabaseGenerator",
        STRING_POKEMON + " Home", "Test Database Generator",
        "",
        "Run all Pokemon Home image readers and detectors against a directory of screenshots.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

TestDatabaseGenerator::TestDatabaseGenerator()
    : DIRECTORY(
        false,
        "<b>Screenshot Directory:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "",
        "Path to a directory containing PNG screenshots."
    )
    , PRINT_DIRECTORY(
        false,
        "<b>Printed Directory Override:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "",
        "Optional directory prefix for logged and generated database paths."
    )
    , RENAME_FILES(
        "<b>Rename Screenshots:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(DIRECTORY);
    PA_ADD_OPTION(PRINT_DIRECTORY);
    PA_ADD_OPTION(RENAME_FILES);
}

namespace{

[[nodiscard]] std::string optional_arrow_name(std::optional<SelectionArrowType> arrow){
    if (!arrow){
        return "none";
    }
    return *arrow == SelectionArrowType::RIGHT ? "right" : "down";
}

[[nodiscard]] std::string to_uppercase(std::string value){
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){
        return static_cast<char>(std::toupper(c));
    });
    return value;
}

[[nodiscard]] std::string quote_string(std::string value){
    return "\"" + value + "\"";
}

[[nodiscard]] std::string boolean(bool value){
    return value ? "true" : "false";
}

}

struct TestEntry{
    std::string box_view;
    std::string summary;
    std::string shiny;
    std::string alpha;
    std::string gigantamax;
    std::string selection_arrow;
    std::string button;
    std::string ball;
    std::string gender;
    std::string tera_type;
    std::string origin_mark;
    std::string summary_numbers;
    std::string summary_text;
    std::string summary_ot_name;
    std::string language_of_origin;
};


void TestDatabaseGenerator::program(
    SingleSwitchProgramEnvironment& env,
    CancellableScope& scope
){
    const Filesystem::Path directory = Filesystem::Path((std::string)DIRECTORY);
    std::vector<Filesystem::Path> screenshots;

    try{
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory.stdpath())){
            if (!entry.is_regular_file())
                continue;

            Filesystem::Path path(entry.path());
            std::string extension = path.extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c){
                return (char)std::tolower(c);
            });
            if (extension == ".png")
                screenshots.emplace_back(std::move(path));
        }
    }catch (const std::filesystem::filesystem_error& error){
        env.log(std::string("Unable to read screenshot directory: ") + error.what(), COLOR_RED);
        return;
    }

    std::sort(screenshots.begin(), screenshots.end(), [](const Filesystem::Path& x, const Filesystem::Path& y){
        return x.string() < y.string();
    });

    std::vector<TestEntry> entries;
    entries.reserve(screenshots.size());

    for (const Filesystem::Path& path : screenshots){
        try{
            ImageRGB32 image(path.string());
            ImageFloatBox arrow_box(0.021, 0.073, 0.47, 0.72);
            ImageFloatBox button_box(0.100, 0.956, 0.107, 0.041);

            SummaryReader summary_reader;
            const int dex_number = summary_reader.read_national_dex(env.console, image);
            const int original_trainer_id = summary_reader.read_original_trainer_id(env.console, image);
            const std::string original_trainer_name = summary_reader.read_original_trainer_name(Language::English, image);
            const std::string nature = summary_reader.read_nature(Language::English, image);
            const std::string ability = summary_reader.read_ability(Language::English, image);
            const int level = summary_reader.read_level(env.console, image);
            BoxViewDetector box_view_detector(&env.console.overlay());
            SummaryScreenDetector summary_detector(&env.console.overlay());
            ShinyDetector shiny_detector(COLOR_RED, &env.console.overlay());
            AlphaDetector alpha_detector(COLOR_RED, &env.console.overlay());
            GigantamaxDetector gigantamax_detector(COLOR_RED, &env.console.overlay());
            SelectionArrowDetector right_arrow_detector(COLOR_RED, &env.console.overlay(), SelectionArrowType::RIGHT, arrow_box);
            SelectionArrowDetector down_arrow_detector(COLOR_RED, &env.console.overlay(), SelectionArrowType::DOWN, arrow_box);
            ButtonDetector b_detector(COLOR_RED, ButtonType::ButtonB, button_box, &env.console.overlay());
            ButtonDetector plus_detector(COLOR_RED, ButtonType::ButtonPlus, button_box, &env.console.overlay());
            BallReader ball_reader(env.console);
            OriginMarkReader origin_reader;

            const bool box_view = box_view_detector.detect(image);
            const bool summary = summary_detector.detect(image);
            const bool shiny = shiny_detector.detect(image);
            const bool alpha = alpha_detector.detect(image);
            const bool gigantamax = gigantamax_detector.detect(image);
            const bool right_arrow = right_arrow_detector.detect(image);
            const bool down_arrow = down_arrow_detector.detect(image);
            const bool button_b = b_detector.detect(image);
            const bool button_plus = plus_detector.detect(image);
            const std::string ball = ball_reader.read_ball(image);
            const std::string language_of_origin = summary_reader.read_language_of_origin(image);
            const Pokemon::StatsHuntGenderFilter gender = BoxGenderDetector::detect(image);
            const Pokemon::PokemonTeraType tera_type = read_pokemon_tera_type(image, { 0.463, 0.09, 0.04, 0.06 });
            const Pokemon::OriginMark origin = origin_reader.read_mark(image);

            std::string renamed_path;
            if (RENAME_FILES){
                if (dex_number <= 0 || dex_number > static_cast<int>(NATIONAL_DEX_SLUGS().size())){
                    env.log(path.string() + " | unable to rename: invalid dex number " + std::to_string(dex_number), COLOR_RED);
                }else{
                    const std::string new_name = NATIONAL_DEX_SLUGS()[dex_number - 1]
                        + "_" + (alpha ? "Alpha" : "Regular")
                        + (shiny ? "_Shiny" : "")
                        + "_" + ORIGIN_MARK_SLUGS().get_string(origin)
                        + ".png";
                    const Filesystem::Path destination = path.parent_path() / new_name;
                    if (destination == path){
                        renamed_path = destination.string();
                    }else if (Filesystem::exists(destination)){
                        env.log(path.string() + " | unable to rename: destination already exists " + destination.string(), COLOR_RED);
                    }else{
                        Filesystem::rename(path, destination);
                        renamed_path = destination.string();
                    }
                }
            }

            std::optional<SelectionArrowType> arrow;
            if (right_arrow)
                arrow = SelectionArrowType::RIGHT;
            else if (down_arrow)
                arrow = SelectionArrowType::DOWN;

            const Filesystem::Path database_file = renamed_path.empty() ? path : Filesystem::Path(renamed_path);
            const std::string print_directory = (std::string)PRINT_DIRECTORY;
            const std::string database_path = print_directory.empty()
                ? database_file.string()
                : print_directory + database_file.filename().string();

            std::ostringstream output;
            output << std::boolalpha
                   << database_path;
            if (!renamed_path.empty()){
                output << " (renamed -> " << renamed_path << ")";
            }
            output << "\n"

                   << "  UI State:       box-view=" << box_view
                   << ", summary=" << summary
                   << ", arrow=" << optional_arrow_name(arrow)
                   << ", B=" << button_b
                   << ", plus=" << button_plus << "\n"

                   << "  Box Indicators: form=" << (alpha ? "Alpha" : "Regular")
                   << ", shiny=" << shiny
                   << ", gigantamax=" << gigantamax
                   << ", gender=" << gender_to_string(gender)
                   << ", ball=" << (ball.empty() ? "none" : ball)
                   << ", tera=" << POKEMON_TERA_TYPE_SLUGS().get_string(tera_type)
                   << ", origin=" << ORIGIN_MARK_SLUGS().get_string(origin) << "\n"

                   << "  Summary Info:   dex=" << dex_number
                   << ", level=" << level
                   << ", language-of-origin=" << (language_of_origin.empty() ? "none" : language_of_origin)
                   << ", nature=" << quote_string(nature)
                   << ", ability=" << quote_string(ability)
                   << ", OT=" << quote_string(original_trainer_name)
                   << ", OT-ID=" << original_trainer_id;
            env.log(output.str());

            TestEntry entry;
            entry.summary_numbers = std::format(
                "database.add<Test_SummaryReader_Numbers>({}, {}, {}, {});",
                quote_string(database_path), dex_number, original_trainer_id, level
            );

            entry.summary_text = std::format(
                "database.add<Test_SummaryReader_Text>({}, {}, {}, Language::English);",
                quote_string(database_path), quote_string(nature), quote_string(ability)
            );

            entry.summary_ot_name = std::format(
                "database.add<Test_SummaryReader_OtName>({}, {}, Language::English);",
                quote_string(database_path), quote_string(original_trainer_name)
            );

            entry.language_of_origin = std::format(
                "database.add<Test_SummaryReader_LanguageOfOrigin>({}, {});",
                quote_string(database_path), quote_string(language_of_origin)
            );

            entry.box_view = std::format(
                "database.add<Test_BoxViewDetector>({}, {});", quote_string(database_path), boolean(box_view)
            );

            entry.summary = std::format(
                "database.add<Test_SummaryScreenDetector>({}, {});", quote_string(database_path), boolean(summary)
            );

            entry.shiny = std::format(
                "database.add<Test_ShinyDetector>({}, {});", quote_string(database_path), boolean(shiny)
            );

            entry.alpha = std::format(
                "database.add<Test_AlphaDetector>({}, {});", quote_string(database_path), boolean(alpha)
            );

            entry.gigantamax = std::format(
                "database.add<Test_GigantamaxDetector>({}, {});", quote_string(database_path), boolean(gigantamax)
            );

            entry.selection_arrow = std::format(
                "database.add<Test_SelectionArrowDetector>({}, {});",
                quote_string(database_path),
                arrow ? "SelectionArrowType::" + to_uppercase(optional_arrow_name(arrow)) : "std::nullopt"
            );

            entry.button = std::format(
                "database.add<Test_ButtonDetector>({}, {});",
                quote_string(database_path),
                button_b ? "ButtonType::ButtonB" : button_plus ? "ButtonType::ButtonPlus" : "std::nullopt"
            );

            entry.ball = std::format(
                "database.add<Test_BallReader>({}, {});", quote_string(database_path), quote_string(ball)
            );

            entry.gender = std::format(
                "database.add<Test_BoxGenderDetector>({}, Pokemon::StatsHuntGenderFilter::{});",
                quote_string(database_path), gender_to_string(gender)
            );

            entry.tera_type = std::format(
                "database.add<Test_TeraTypeReader>({}, PokemonTeraType::{});",
                quote_string(database_path), to_uppercase(POKEMON_TERA_TYPE_SLUGS().get_string(tera_type))
            );

            entry.origin_mark = std::format(
                "database.add<Test_OriginMarkReader>({}, OriginMark::{});",
                quote_string(database_path), to_uppercase(ORIGIN_MARK_SLUGS().get_string(origin))
            );

            entries.push_back(std::move(entry));
        }catch (const std::exception& error){
            env.log(path.string() + " | error: " + error.what(), COLOR_RED);
        }
    }

    struct Section{
        const char* name;
        std::string TestEntry::* member;
    };
    static constexpr Section SECTIONS[] = {
        {"BoxViewDetector",         &TestEntry::box_view},
        {"SummaryScreenDetector",   &TestEntry::summary},
        {"ShinyDetector",           &TestEntry::shiny},
        {"AlphaDetector",           &TestEntry::alpha},
        {"GigantamaxDetector",      &TestEntry::gigantamax},
        {"SelectionArrowDetector",  &TestEntry::selection_arrow},
        {"ButtonDetector",          &TestEntry::button},
        {"BallReader",              &TestEntry::ball},
        {"BoxGenderDetector",       &TestEntry::gender},
        {"TeraTypeReader",          &TestEntry::tera_type},
        {"OriginMarkReader",        &TestEntry::origin_mark},
        {"SummaryReader_Numbers",   &TestEntry::summary_numbers},
        {"SummaryReader_Text",      &TestEntry::summary_text},
        {"SummaryReader_OtName",    &TestEntry::summary_ot_name},
        {"SummaryReader_LanguageOfOrigin",& TestEntry::language_of_origin}
    };

    const Filesystem::Path output_path = directory / "PokemonHome_TestDatabase.txt";
    std::ofstream output_file(output_path.string());
    if (!output_file){
        env.log("Unable to write database output file: " + output_path.string(), COLOR_RED);
        return;
    }

    for (const Section& section : SECTIONS){
        output_file << section.name << ":\n";
        for (const TestEntry& entry : entries){
            output_file << entry.*section.member << "\n";
        }
    }
}

}
}
}
