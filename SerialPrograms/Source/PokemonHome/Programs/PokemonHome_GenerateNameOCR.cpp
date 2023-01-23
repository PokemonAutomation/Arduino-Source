/*  Pokemon Home Generate Name OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDir>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonHome_GenerateNameOCR.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
using namespace Pokemon;


GenerateNameOCRData_Descriptor::GenerateNameOCRData_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonHome:GenerateNameOCR",
        STRING_POKEMON + " Home", STRING_POKEMON + " Home: Generate Name OCR",
        "",
        "Generate " + STRING_POKEMON + " Name OCR data by iterating the National " + STRING_POKEDEX + ".",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



GenerateNameOCRData::GenerateNameOCRData()
    : LANGUAGE(
        "<b>Game Language:</b>",
        PokemonNameReader::instance().languages(),
        LockWhileRunning::LOCKED
    )
    , DELAY(
        "<b>Delay Between Each Iteration:</b>",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "30"
    )
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(DELAY);

}


void GenerateNameOCRData::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    std::string resource_path = RESOURCE_PATH() + "Pokemon/Pokedex/Pokedex-National.json";
    JsonValue json = load_json_file(resource_path);
    JsonArray& array = json.get_array_throw(resource_path);

    std::vector<std::string> slugs;
    for (auto& item : array){
        std::string& slug = item.get_string_throw(resource_path);
        slugs.emplace_back(std::move(slug));
    }


    OverlayBoxScope box(env.console, {0.705, 0.815, 0.219, 0.055});
    std::string language_code = language_data(LANGUAGE).code;

    for (const std::string& slug : slugs){
        context.wait_for_all_requests();

        VideoSnapshot screen = env.console.video().snapshot();
        ImageViewRGB32 image = extract_box_reference(screen, box);

        std::string path = "PokemonNameOCR/";
        path += language_code;
        path += "/";

        QDir dir(QString::fromStdString(path));
        if (!dir.exists()){
            dir.mkpath(".");
        }

        path += slug;
        path += "-";
        path += now_to_filestring();
        path += ".png";
        image.save(path);

        pbf_press_dpad(context, DPAD_RIGHT, 10, DELAY);

        OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(
            env.console, LANGUAGE, image,
            OCR::BLACK_TEXT_FILTERS()
        );
    }


}




}
}
}
