/*  Pokemon Home Generate Name OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDir>
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/OCR/Filtering.h"
#include "PokemonHome_GenerateNameOCR.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


GenerateNameOCRData_Descriptor::GenerateNameOCRData_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonHome:GenerateNameOCR",
        STRING_POKEMON + " Home: Generate Name OCR",
        "",
        "Generate " + STRING_POKEMON + " Name OCR data by iterating the National " + STRING_POKEDEX + ".",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



GenerateNameOCRData::GenerateNameOCRData(const GenerateNameOCRData_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE(
        "<b>Game Language:</b>",
        m_reader.languages()
    )
    , DELAY(
        "<b>Delay Between Each Iteration:</b>",
        "30"
    )
{
    m_options.emplace_back(&LANGUAGE, "LANGUAGE");
    m_options.emplace_back(&DELAY, "DELAY");
}


void GenerateNameOCRData::program(SingleSwitchProgramEnvironment& env){

    QJsonArray array = read_json_file(
        PERSISTENT_SETTINGS().resource_path + "Pokemon/Pokedex/Pokedex-National.json"
    ).array();

    std::vector<std::string> tokens;
    for (const auto& item : array){
        QString token = item.toString();
        if (token.size() <= 0){
            PA_THROW_StringException("Expected non-empty string for Pokemon token.");
        }
        tokens.emplace_back(token.toUtf8().data());
    }


    InferenceBoxScope box(env.console, 0.705, 0.815, 0.219, 0.055);
    QString language_code = language_data(LANGUAGE).code.c_str();

    for (const std::string& token : tokens){
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

        path += token.c_str();
        path += "-";
        path += now_to_filestring().c_str();
        path += ".png";
        image.save(path);

        pbf_press_dpad(env.console, DPAD_RIGHT, 10, DELAY);

        OCR::make_OCR_filter(image).apply(image);

        OCR::MatchResult result = m_reader.read_exact(LANGUAGE, token, image);
        result.log(&env.logger());
    }


}




}
}
}
