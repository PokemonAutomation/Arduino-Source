/*  Generate Location Name OCR Data
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
#include "PokemonLZA/Inference/Map/PokemonLZA_LocationNameReader.h"
#include "PokemonLZA_GenerateLocationNameOCR.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{
using namespace Pokemon;


GenerateLocationNameOCR_Descriptor::GenerateLocationNameOCR_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:GenerateLocationNameOCR",
        STRING_POKEMON + " LZA", "Generate Location Name OCR Data",
        "",
        "Generate Location Name OCR data by iterating the fly spot list.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}



GenerateLocationNameOCR::GenerateLocationNameOCR()
    : LANGUAGE(
        "<b>Game Language:</b>",
        LocationNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING
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
    PA_ADD_OPTION(MODE);
}

void GenerateLocationNameOCR::read(
    JsonArray& output,
    Logger& logger,
    const ImageViewRGB32& image
) const{
    OCR::StringMatchResult result = LocationNameReader::instance().read_substring(
        logger, LANGUAGE, image,
        OCR::BLACK_OR_WHITE_TEXT_FILTERS()
    );
    if (result.results.empty()){
        output.push_back("");
    }else{
        output.push_back(result.results.begin()->second.token);
    }
}

void GenerateLocationNameOCR::dump_images(
    const std::vector<std::string>& expected,
    size_t index,
    const ImageViewRGB32& image
) const{
    if (index >= expected.size()){
        return;
    }

    std::string path = "LocationNameOCR/";
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

void GenerateLocationNameOCR::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    size_t location_num = 67;

    OverlayBoxScope box0(env.console, {0.035000, 0.254000 + 0 * 0.078000, 0.287000, 0.050000}, COLOR_BLUE);
    OverlayBoxScope box1(env.console, {0.035000, 0.254000 + 1 * 0.078000, 0.287000, 0.050000}, COLOR_BLUE);
    OverlayBoxScope box2(env.console, {0.035000, 0.254000 + 2 * 0.078000, 0.287000, 0.050000}, COLOR_BLUE);
    OverlayBoxScope box3(env.console, {0.035000, 0.254000 + 3 * 0.078000, 0.287000, 0.050000}, COLOR_BLUE);
    OverlayBoxScope box4(env.console, {0.035000, 0.254000 + 4 * 0.078000, 0.287000, 0.050000}, COLOR_BLUE);
    OverlayBoxScope box5(env.console, {0.035000, 0.254000 + 5 * 0.078000, 0.287000, 0.050000}, COLOR_BLUE);
    OverlayBoxScope box6(env.console, {0.035000, 0.254000 + 6 * 0.078000, 0.287000, 0.050000}, COLOR_BLUE);

    std::vector<std::string> expected;
    JsonArray actual;
//    OCR::DictionaryOCR& dictionary = m_reader.dictionary(LANGUAGE);

    if (MODE == Mode::GenerateTrainingData){
        std::string path = RESOURCE_PATH() + "PokemonLZA/LocationName.json";
        JsonValue json = load_json_file(path);
        JsonArray& array = json.to_array_throw(path);
        for (const auto& item : array){
            expected.emplace_back(item.to_string_throw(path));
        }
    }

    for (size_t c = 1; c <= location_num; c += 7){
        context.wait_for_all_requests();

        if (c + 6 > location_num){
            c = location_num - 6;
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

        pbf_press_dpad(context, DPAD_RIGHT, 160ms, 80ms);
    }

    if (MODE == Mode::SaveToJson){
        actual.dump("LocationName.json");
    }

}



}
}
}
