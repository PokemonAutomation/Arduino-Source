/*  Generate Pokemon Name OCR Data (Pokedex)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDir>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh_GenerateNameOCRPokedex.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;


GenerateNameOCRDataPokedex_Descriptor::GenerateNameOCRDataPokedex_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:GenerateNameOCRPokedex",
        STRING_POKEMON + " SwSh", "Generate " + STRING_POKEMON + " Name OCR Data",
        "",
        "Generate " + STRING_POKEMON + " Name OCR data by iterating the " + STRING_POKEDEX + ".",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



GenerateNameOCRDataPokedex::GenerateNameOCRDataPokedex(const GenerateNameOCRDataPokedex_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE(
        "<b>Game Language:</b>",
        PokemonNameReader::instance().languages()
    )
    , POKEDEX(
        "<b>" + STRING_POKEDEX + ":</b>",
        {
            "Galar",
            "Isle of Armor",
            "Crown Tundra",
        },
        0
    )
    , MODE(
        "<b>Mode:</b>",
        {
            "Read names and save to JSON.",
            "Generate training data.",
        },
        1
    )
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(POKEDEX);
    PA_ADD_OPTION(MODE);
}

void GenerateNameOCRDataPokedex::read(
    JsonArray& output,
    LoggerQt& logger,
    QImage image
) const{
    OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(
        logger, LANGUAGE, image,
        OCR::BLACK_OR_WHITE_TEXT_FILTERS()
    );
    if (result.results.empty()){
        output.push_back("");
    }else{
        output.push_back(result.results.begin()->second.token);
    }
}

void GenerateNameOCRDataPokedex::dump_images(
    const std::vector<std::string>& expected,
    size_t index,
    const QImage& image
) const{
    if (index >= expected.size()){
        return;
    }

    std::string path = "PokemonNameOCR/";
    path += language_data(LANGUAGE).code;

    QDir dir(QString::fromStdString(path));
    if (!dir.exists()){
        dir.mkpath(".");
    }

    path += "/";
    path += expected[index];
    path += "-";
    path += now_to_filestring();
    path += ".png";
    image.save(QString::fromStdString(path));

//    OCR::make_OCR_filter(image).apply(image);
}

void GenerateNameOCRDataPokedex::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    std::string dex_name;
    size_t dex_size = 0;
    switch (POKEDEX){
    case 0:
        dex_name = "Galar";
        dex_size = 400;
        break;
    case 1:
        dex_name = "IsleOfArmor";
        dex_size = 211;
        break;
    case 2:
        dex_name = "CrownTundra";
        dex_size = 210;
        break;
    }

    InferenceBoxScope box0(env.console, 0.75, 0.146 + 0 * 0.1115, 0.18, 0.059, COLOR_BLUE);
    InferenceBoxScope box1(env.console, 0.75, 0.146 + 1 * 0.1115, 0.18, 0.059, COLOR_BLUE);
    InferenceBoxScope box2(env.console, 0.75, 0.146 + 2 * 0.1115, 0.18, 0.059, COLOR_BLUE);
    InferenceBoxScope box3(env.console, 0.75, 0.146 + 3 * 0.1115, 0.18, 0.059, COLOR_BLUE);
    InferenceBoxScope box4(env.console, 0.75, 0.146 + 4 * 0.1115, 0.18, 0.059, COLOR_BLUE);
    InferenceBoxScope box5(env.console, 0.75, 0.146 + 5 * 0.1115, 0.18, 0.059, COLOR_BLUE);
    InferenceBoxScope box6(env.console, 0.75, 0.146 + 6 * 0.1115, 0.18, 0.059, COLOR_BLUE);

    std::vector<std::string> expected;
    JsonArray actual;
//    OCR::DictionaryOCR& dictionary = m_reader.dictionary(LANGUAGE);

    if (MODE == Mode::GENERATE_TRAINING_DATA){
        std::string path = RESOURCE_PATH() + "Pokemon/Pokedex/Pokedex-" + dex_name + ".json";
        JsonValue json = load_json_file(path);
        JsonArray& array = json.get_array_throw(path);
        for (const auto& item : array){
            expected.emplace_back(item.get_string_throw(path));
        }
    }

    for (size_t c = 1; c <= dex_size; c += 7){
        context.wait_for_all_requests();

        if (c + 6 > dex_size){
            c = dex_size - 6;
        }
//        cout << "dex: " << c << endl;

        QImage frame = env.console.video().snapshot();
        QImage image0 = extract_box_copy(frame, box0);
        QImage image1 = extract_box_copy(frame, box1);
        QImage image2 = extract_box_copy(frame, box2);
        QImage image3 = extract_box_copy(frame, box3);
        QImage image4 = extract_box_copy(frame, box4);
        QImage image5 = extract_box_copy(frame, box5);
        QImage image6 = extract_box_copy(frame, box6);

//        image1.save("test.png");

        switch (MODE){
        case Mode::READ_AND_SAVE:
            read(actual, env.console, std::move(image0));
            read(actual, env.console, std::move(image1));
            read(actual, env.console, std::move(image2));
            read(actual, env.console, std::move(image3));
            read(actual, env.console, std::move(image4));
            read(actual, env.console, std::move(image5));
            read(actual, env.console, std::move(image6));
            break;
        case Mode::GENERATE_TRAINING_DATA:
            dump_images(expected, c - 1 + 0, image0);
            dump_images(expected, c - 1 + 1, image1);
            dump_images(expected, c - 1 + 2, image2);
            dump_images(expected, c - 1 + 3, image3);
            dump_images(expected, c - 1 + 4, image4);
            dump_images(expected, c - 1 + 5, image5);
            dump_images(expected, c - 1 + 6, image6);
            break;
        }

        pbf_press_dpad(context, DPAD_RIGHT, 10, TICKS_PER_SECOND);
    }

    if (MODE == Mode::READ_AND_SAVE){
        actual.dump("PokedexReadData.json");
    }

}



}
}
}
