/*  Generate IV Checker OCR Data
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDir>
#include <QImage>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh_GenerateIVCheckerOCR.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const std::string IVCheckerOptionOCR::TOKENS[]{
    "No Good",
    "Decent",
    "Pretty Good",
    "Very Good",
    "Fantastic",
    "Best",
    "Hyper trained!",
};



GenerateIVCheckerOCR_Descriptor::GenerateIVCheckerOCR_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:GenerateIVCheckerOCR",
        STRING_POKEMON + " SwSh", "Generate IV Checker OCR Data",
        "",
        "Generate IV Checker OCR Data",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



GenerateIVCheckerOCR::GenerateIVCheckerOCR(const GenerateIVCheckerOCR_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE(
        "<b>Game Language:</b>",
        IVCheckerReader::instance().languages()
    )
    , HP("<b>HP:</b>")
    , ATTACK("<b>Attack:</b>")
    , DEFENSE("<b>Defense:</b>")
    , SPATK("<b>Sp. Atk:</b>")
    , SPDEF("<b>Sp. Def:</b>")
    , SPEED("<b>Speed:</b>")
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(HP);
    PA_ADD_OPTION(ATTACK);
    PA_ADD_OPTION(DEFENSE);
    PA_ADD_OPTION(SPATK);
    PA_ADD_OPTION(SPDEF);
    PA_ADD_OPTION(SPEED);
}


void GenerateIVCheckerOCR::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    IVCheckerReaderScope reader(env.console, LANGUAGE);

    std::string path = "IVCheckerOCR/";
    path += language_data(LANGUAGE).code;

    QDir dir(QString::fromStdString(path));
    if (!dir.exists()){
        dir.mkpath(".");
    }
    path += "/";

    std::vector<QImage> images = reader.dump_images(env.console.video().snapshot());

    std::string now = now_to_filestring();
    images[0].save(QString::fromStdString(path + IVCheckerOptionOCR::TOKENS[HP] + "-" + now + "a.png"));
    images[1].save(QString::fromStdString(path + IVCheckerOptionOCR::TOKENS[ATTACK] + "-" + now + "b.png"));
    images[2].save(QString::fromStdString(path + IVCheckerOptionOCR::TOKENS[DEFENSE] + "-" + now + "c.png"));
    images[3].save(QString::fromStdString(path + IVCheckerOptionOCR::TOKENS[SPATK] + "-" + now + "d.png"));
    images[4].save(QString::fromStdString(path + IVCheckerOptionOCR::TOKENS[SPDEF] + "-" + now + "e.png"));
    images[5].save(QString::fromStdString(path + IVCheckerOptionOCR::TOKENS[SPEED] + "-" + now + "f.png"));

}



}
}
}

