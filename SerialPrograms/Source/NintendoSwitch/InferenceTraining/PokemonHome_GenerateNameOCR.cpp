/*  Pokemon Home Generate Name OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDir>
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/OCR/OCR_Filtering.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonHome_GenerateNameOCR.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
using namespace Pokemon;


GenerateNameOCRData_Descriptor::GenerateNameOCRData_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonHome:GenerateNameOCR",
        STRING_POKEMON + " Home", STRING_POKEMON + " Home: Generate Name OCR",
        "",
        "Generate " + STRING_POKEMON + " Name OCR data by iterating the National " + STRING_POKEDEX + ".",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



GenerateNameOCRData::GenerateNameOCRData(const GenerateNameOCRData_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE(
        "<b>Game Language:</b>",
        PokemonNameReader::instance().languages()
    )
    , DELAY(
        "<b>Delay Between Each Iteration:</b>",
        "30"
    )
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(DELAY);

}


void GenerateNameOCRData::program(SingleSwitchProgramEnvironment& env){

    QJsonArray array = read_json_file(
        RESOURCE_PATH() + "Pokemon/Pokedex/Pokedex-National.json"
    ).array();

    std::vector<std::string> slugs;
    for (const auto& item : array){
        QString slug = item.toString();
        if (slug.size() <= 0){
            PA_THROW_StringException("Expected non-empty string for Pokemon slug.");
        }
        slugs.emplace_back(slug.toUtf8().data());
    }


    InferenceBoxScope box(env.console, 0.705, 0.815, 0.219, 0.055);
    QString language_code = QString::fromStdString(language_data(LANGUAGE).code);

    for (const std::string& slug : slugs){
        env.console.botbase().wait_for_all_requests();

        QImage screen = env.console.video().snapshot();
        QImage image = extract_box(screen, box);

        QString path = "PokemonNameOCR/";
        path += language_code;
        path += "/";

        QDir dir(path);
        if (!dir.exists()){
            dir.mkpath(".");
        }

        path += QString::fromStdString(slug);
        path += "-";
        path += QString::fromStdString(now_to_filestring());
        path += ".png";
        image.save(path);

        pbf_press_dpad(env.console, DPAD_RIGHT, 10, DELAY);

        OCR::make_OCR_filter(image).apply(image);

        OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(env.console, LANGUAGE, image);
    }


}




}
}
}
