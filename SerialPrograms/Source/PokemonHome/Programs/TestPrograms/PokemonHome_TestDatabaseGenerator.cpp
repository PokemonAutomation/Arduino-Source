/*  Pokemon Home Test Database Generator
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
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

std::string optional_arrow_name(std::optional<SelectionArrowType> arrow){
    if (!arrow)
        return "none";
    return *arrow == SelectionArrowType::RIGHT ? "right" : "down";
}

std::string uppercase(std::string value){
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){
        return (char)std::toupper(c);
    });
    return value;
}

std::string quoted(const std::string& value){
    return "\"" + value + "\"";
}

std::string boolean(bool value){
    return value ? "true" : "false";
}

}

void TestDatabaseGenerator::program(
    SingleSwitchProgramEnvironment& env,
    CancellableScope& scope
){
    const Filesystem::Path directory = Filesystem::Path((std::string)DIRECTORY);
    std::vector<Filesystem::Path> screenshots;
    std::vector<std::string> summary_reader_numbers_tests;
    std::vector<std::string> summary_reader_text_tests;
    std::vector<std::string> summary_reader_ot_name_tests;
    std::vector<std::string> box_view_tests;
    std::vector<std::string> summary_tests;
    std::vector<std::string> shiny_tests;
    std::vector<std::string> alpha_tests;
    std::vector<std::string> gigantamax_tests;
    std::vector<std::string> selection_arrow_tests;
    std::vector<std::string> button_tests;
    std::vector<std::string> ball_tests;
    std::vector<std::string> gender_tests;
    std::vector<std::string> tera_type_tests;
    std::vector<std::string> origin_mark_tests;

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

    for (const Filesystem::Path& path : screenshots){
        try{
            ImageRGB32 image(path.string());
            ImageFloatBox box(0.463, 0.09, 0.04, 0.06);

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
            GigantamaxDetector gigantamax_detector(COLOR_RED, &env.console.overlay(), box);
            SelectionArrowDetector right_arrow_detector(COLOR_RED, &env.console.overlay(), SelectionArrowType::RIGHT, box);
            SelectionArrowDetector down_arrow_detector(COLOR_RED, &env.console.overlay(), SelectionArrowType::DOWN, box);
            ButtonDetector b_detector(COLOR_RED, ButtonType::ButtonB, ImageFloatBox(0.100, 0.956, 0.107, 0.041), &env.console.overlay());
            ButtonDetector plus_detector(COLOR_RED, ButtonType::ButtonPlus, ImageFloatBox(0.100, 0.956, 0.107, 0.041), &env.console.overlay());
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
            const Pokemon::StatsHuntGenderFilter gender = BoxGenderDetector::detect(image);
            const Pokemon::PokemonTeraType tera_type = read_pokemon_tera_type(image, box);
            const Pokemon::OriginMark origin = origin_reader.read_mark(image);

            std::string renamed_path;
            if (RENAME_FILES){
                if (dex_number <= 0 || dex_number > (int)NATIONAL_DEX_SLUGS().size()){
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
                   << database_path
                   << " | dex=" << dex_number
                   << ", form=" << (alpha ? "Alpha" : "Regular")
                   << ", shiny=" << shiny
                   << " | box-view=" << box_view
                   << ", summary=" << summary
                   << ", gigantamax=" << gigantamax
                   << ", arrow=" << optional_arrow_name(arrow)
                   << ", B=" << button_b
                   << ", plus=" << button_plus
                   << ", ball=" << (ball.empty() ? "none" : ball)
                   << ", gender=" << gender_to_string(gender)
                   << ", tera=" << POKEMON_TERA_TYPE_SLUGS().get_string(tera_type)
                   << ", origin=" << ORIGIN_MARK_SLUGS().get_string(origin)
                   << " | OT=" << quoted(original_trainer_name)
                   << ", OT-ID=" << original_trainer_id
                   << ", nature=" << quoted(nature)
                   << ", ability=" << quoted(ability)
                   << ", level=" << level;
            if (!renamed_path.empty())
                output << " | renamed-to=" << renamed_path;
            env.log(output.str());

            summary_reader_numbers_tests.emplace_back(
                "database.add<Test_SummaryReader_Numbers>(" + quoted(database_path) + ", "
                + std::to_string(dex_number) + ", " + std::to_string(original_trainer_id) + ", "
                + std::to_string(level) + ");"
            );
            summary_reader_text_tests.emplace_back(
                "database.add<Test_SummaryReader_Text>(" + quoted(database_path) + ", "
                + quoted(nature) + ", " + quoted(ability) + ", Language::English);"
            );
            summary_reader_ot_name_tests.emplace_back(
                "database.add<Test_SummaryReader_OtName>(" + quoted(database_path) + ", "
                + quoted(original_trainer_name) + ", Language::English);"
            );
            box_view_tests.emplace_back(
                "database.add<Test_BoxViewDetector>(" + quoted(database_path) + ", " + boolean(box_view) + ");"
            );
            summary_tests.emplace_back(
                "database.add<Test_SummaryScreenDetector>(" + quoted(database_path) + ", " + boolean(summary) + ");"
            );
            shiny_tests.emplace_back(
                "database.add<Test_ShinyDetector>(" + quoted(database_path) + ", " + boolean(shiny) + ");"
            );
            alpha_tests.emplace_back(
                "database.add<Test_AlphaDetector>(" + quoted(database_path) + ", " + boolean(alpha) + ");"
            );
            gigantamax_tests.emplace_back(
                "database.add<Test_GigantamaxDetector>(" + quoted(database_path) + ", " + boolean(gigantamax) + ");"
            );
            selection_arrow_tests.emplace_back(
                "database.add<Test_SelectionArrowDetector>(" + quoted(database_path) + ", "
                + (arrow ? "SelectionArrowType::" + uppercase(optional_arrow_name(arrow)) : "std::nullopt") + ");"
            );
            button_tests.emplace_back(
                "database.add<Test_ButtonDetector>(" + quoted(database_path) + ", "
                + (button_b ? "ButtonType::ButtonB" : button_plus ? "ButtonType::ButtonPlus" : "std::nullopt") + ");"
            );
            ball_tests.emplace_back(
                "database.add<Test_BallReader>(" + quoted(database_path) + ", " + quoted(ball) + ");"
            );
            gender_tests.emplace_back(
                "database.add<Test_BoxGenderDetector>(" + quoted(database_path) + ", Pokemon::StatsHuntGenderFilter::"
                + gender_to_string(gender) + ");"
            );
            tera_type_tests.emplace_back(
                "database.add<Test_TeraTypeReader>(" + quoted(database_path) + ", PokemonTeraType::"
                + uppercase(POKEMON_TERA_TYPE_SLUGS().get_string(tera_type)) + ");"
            );
            origin_mark_tests.emplace_back(
                "database.add<Test_OriginMarkReader>(" + quoted(database_path) + ", OriginMark::"
                + uppercase(ORIGIN_MARK_SLUGS().get_string(origin)) + ");"
            );
        }catch (const std::exception& error){
            env.log(path.string() + " | error: " + error.what(), COLOR_RED);
        }
    }

    auto log_database = [&](const char* name, const std::vector<std::string>& entries){
        env.log(std::string("\n") + name + ":");
        for (const std::string& entry : entries)
            env.log(entry);
    };
    log_database("BoxViewDetector", box_view_tests);
    log_database("SummaryScreenDetector", summary_tests);
    log_database("ShinyDetector", shiny_tests);
    log_database("AlphaDetector", alpha_tests);
    log_database("GigantamaxDetector", gigantamax_tests);
    log_database("SelectionArrowDetector", selection_arrow_tests);
    log_database("ButtonDetector", button_tests);
    log_database("BallReader", ball_tests);
    log_database("BoxGenderDetector", gender_tests);
    log_database("TeraTypeReader", tera_type_tests);
    log_database("OriginMarkReader", origin_mark_tests);
    log_database("SummaryReader_Numbers", summary_reader_numbers_tests);
    log_database("SummaryReader_Text", summary_reader_text_tests);
    log_database("SummaryReader_OtName", summary_reader_ot_name_tests);

    const Filesystem::Path output_path = directory / "PokemonHome_TestDatabase.txt";
    std::ofstream output_file(output_path.string());
    if (!output_file){
        env.log("Unable to write database output file: " + output_path.string(), COLOR_RED);
        return;
    }

    auto write_database = [&](const char* name, const std::vector<std::string>& entries){
        output_file << name << ":\n";
        for (const std::string& entry : entries)
            output_file << entry << "\n";
    };
    write_database("BoxViewDetector", box_view_tests);
    write_database("SummaryScreenDetector", summary_tests);
    write_database("ShinyDetector", shiny_tests);
    write_database("AlphaDetector", alpha_tests);
    write_database("GigantamaxDetector", gigantamax_tests);
    write_database("SelectionArrowDetector", selection_arrow_tests);
    write_database("ButtonDetector", button_tests);
    write_database("BallReader", ball_tests);
    write_database("BoxGenderDetector", gender_tests);
    write_database("TeraTypeReader", tera_type_tests);
    write_database("OriginMarkReader", origin_mark_tests);
    write_database("SummaryReader_Numbers", summary_reader_numbers_tests);
    write_database("SummaryReader_Text", summary_reader_text_tests);
    write_database("SummaryReader_OtName", summary_reader_ot_name_tests);
}

}
}
}
