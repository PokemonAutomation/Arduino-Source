/*  Generate Pokemon Name OCR Data (Pokedex)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QDir>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
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
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:GenerateNameOCRPokedex",
        STRING_POKEMON + " SwSh", "Generate " + STRING_POKEMON + " Name OCR Data",
        "",
        "Generate " + STRING_POKEMON + " Name OCR data by iterating the " + STRING_POKEDEX + ".",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



GenerateNameOCRDataPokedex::GenerateNameOCRDataPokedex()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING
    )
    , POKEDEX(
        "<b>" + STRING_POKEDEX + ":</b>",
        {
            {Pokedex::Galar, "galar", "Galar"},
            {Pokedex::IsleOfArmor, "isle-of-armor", "Isle of Armor"},
            {Pokedex::CrownTundra, "crown-tundra", "Crown Tundra"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Pokedex::Galar
    )
    , MODE(
        "<b>Mode:</b>",
        {
            {Mode::SaveToJson, "save-to-json", "Read names and save to JSON."},
            {Mode::GenerateTrainingData, "generate-training-data", "Generate training data."},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Mode::GenerateTrainingData
    )
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(POKEDEX);
    PA_ADD_OPTION(MODE);
}

void GenerateNameOCRDataPokedex::read(
    JsonArray& output,
    Logger& logger,
    const ImageViewRGB32& image
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
    const ImageViewRGB32& image
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
    image.save(path);

//    OCR::make_OCR_filter(image).apply(image);
}

void GenerateNameOCRDataPokedex::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    std::string dex_name;
    size_t dex_size = 0;
    switch (POKEDEX){
    case Pokedex::Galar:
        dex_name = "Galar";
        dex_size = 400;
        break;
    case Pokedex::IsleOfArmor:
        dex_name = "IsleOfArmor";
        dex_size = 211;
        break;
    case Pokedex::CrownTundra:
        dex_name = "CrownTundra";
        dex_size = 210;
        break;
    }

    OverlayBoxScope box0(env.console, {0.75, 0.146 + 0 * 0.1115, 0.18, 0.059}, COLOR_BLUE);
    OverlayBoxScope box1(env.console, {0.75, 0.146 + 1 * 0.1115, 0.18, 0.059}, COLOR_BLUE);
    OverlayBoxScope box2(env.console, {0.75, 0.146 + 2 * 0.1115, 0.18, 0.059}, COLOR_BLUE);
    OverlayBoxScope box3(env.console, {0.75, 0.146 + 3 * 0.1115, 0.18, 0.059}, COLOR_BLUE);
    OverlayBoxScope box4(env.console, {0.75, 0.146 + 4 * 0.1115, 0.18, 0.059}, COLOR_BLUE);
    OverlayBoxScope box5(env.console, {0.75, 0.146 + 5 * 0.1115, 0.18, 0.059}, COLOR_BLUE);
    OverlayBoxScope box6(env.console, {0.75, 0.146 + 6 * 0.1115, 0.18, 0.059}, COLOR_BLUE);

    std::vector<std::string> expected;
    JsonArray actual;
//    OCR::DictionaryOCR& dictionary = m_reader.dictionary(LANGUAGE);

    if (MODE == Mode::GenerateTrainingData){
        std::string path = RESOURCE_PATH() + "Pokemon/Pokedex/Pokedex-" + dex_name + ".json";
        JsonValue json = load_json_file(path);
        JsonArray& array = json.to_array_throw(path);
        for (const auto& item : array){
            expected.emplace_back(item.to_string_throw(path));
        }
    }

    for (size_t c = 1; c <= dex_size; c += 7){
        context.wait_for_all_requests();

        if (c + 6 > dex_size){
            c = dex_size - 6;
        }
//        cout << "dex: " << c << endl;

        VideoSnapshot frame = env.console.video().snapshot();
        ImageViewRGB32 image0 = extract_box_reference(frame, box0);
        ImageViewRGB32 image1 = extract_box_reference(frame, box1);
        ImageViewRGB32 image2 = extract_box_reference(frame, box2);
        ImageViewRGB32 image3 = extract_box_reference(frame, box3);
        ImageViewRGB32 image4 = extract_box_reference(frame, box4);
        ImageViewRGB32 image5 = extract_box_reference(frame, box5);
        ImageViewRGB32 image6 = extract_box_reference(frame, box6);

//        image1.save("test.png");

        switch (MODE){
        case Mode::SaveToJson:
            read(actual, env.console, image0);
            read(actual, env.console, image1);
            read(actual, env.console, image2);
            read(actual, env.console, image3);
            read(actual, env.console, image4);
            read(actual, env.console, image5);
            read(actual, env.console, image6);
            break;
        case Mode::GenerateTrainingData:
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

    if (MODE == Mode::SaveToJson){
        actual.dump("PokedexReadData.json");
    }

}



}
}
}
