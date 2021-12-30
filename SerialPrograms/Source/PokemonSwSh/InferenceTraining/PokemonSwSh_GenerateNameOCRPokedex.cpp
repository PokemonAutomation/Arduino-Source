/*  Generate Pokemon Name OCR Data (Pokedex)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDir>
#include <QJsonArray>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_Filtering.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
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
        FeedbackType::REQUIRED,
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
    QJsonArray& output,
    Logger& logger,
    QImage image
) const{
    OCR::make_OCR_filter(image).apply(image);
//    image.save("test.png");

    OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(logger, LANGUAGE, image);
    if (result.results.empty()){
        output.append("");
    }else{
        output.append(QString::fromStdString(result.results.begin()->second.token));
    }
}

void GenerateNameOCRDataPokedex::dump_images(
    const std::vector<std::string>& expected,
    size_t index,
    QImage image
) const{
    if (index >= expected.size()){
        return;
    }

    QString path = "PokemonNameOCR/";
    path += QString::fromStdString(language_data(LANGUAGE).code);

    QDir dir(path);
    if (!dir.exists()){
        dir.mkpath(".");
    }

    path += "/";
    path += QString::fromStdString(expected[index]);
    path += "-";
    path += QString::fromStdString(now_to_filestring());
    path += ".png";
    image.save(path);

    OCR::make_OCR_filter(image).apply(image);
}

void GenerateNameOCRDataPokedex::program(SingleSwitchProgramEnvironment& env){

    QString dex_name;
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
    QJsonArray actual;
//    OCR::DictionaryOCR& dictionary = m_reader.dictionary(LANGUAGE);

    if (MODE == Mode::GENERATE_TRAINING_DATA){
        QJsonArray array = read_json_file(
            RESOURCE_PATH() + "Pokemon/Pokedex/Pokedex-" + dex_name + ".json"
        ).array();
        for (const auto& item : array){
            expected.emplace_back(item.toString().toUtf8().data());
        }
    }

    for (size_t c = 1; c <= dex_size; c += 7){
        env.console.botbase().wait_for_all_requests();

        if (c + 6 > dex_size){
            c = dex_size - 6;
        }
//        cout << "dex: " << c << endl;

        QImage frame = env.console.video().snapshot();
        QImage image0 = extract_box(frame, box0);
        QImage image1 = extract_box(frame, box1);
        QImage image2 = extract_box(frame, box2);
        QImage image3 = extract_box(frame, box3);
        QImage image4 = extract_box(frame, box4);
        QImage image5 = extract_box(frame, box5);
        QImage image6 = extract_box(frame, box6);

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
            dump_images(expected, c - 1 + 0, std::move(image0));
            dump_images(expected, c - 1 + 1, std::move(image1));
            dump_images(expected, c - 1 + 2, std::move(image2));
            dump_images(expected, c - 1 + 3, std::move(image3));
            dump_images(expected, c - 1 + 4, std::move(image4));
            dump_images(expected, c - 1 + 5, std::move(image5));
            dump_images(expected, c - 1 + 6, std::move(image6));
            break;
        }

        pbf_press_dpad(env.console, DPAD_RIGHT, 10, TICKS_PER_SECOND);
    }

    if (MODE == Mode::READ_AND_SAVE){
        write_json_file("PokedexReadData.json", QJsonDocument(actual));
    }

}



}
}
}
