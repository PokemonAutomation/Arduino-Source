/*  Generate IV Checker OCR Data
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDir>
#include "Common/Cpp/PrettyPrint.h"
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
        "Generate IV Checker OCR Data",
        "",
        "Generate IV Checker OCR Data",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



GenerateIVCheckerOCR::GenerateIVCheckerOCR(const GenerateIVCheckerOCR_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE(
        "<b>Game Language:</b>",
        m_reader.languages()
    )
    , HP("<b>HP:</b>")
    , ATTACK("<b>Attack:</b>")
    , DEFENSE("<b>Defense:</b>")
    , SPATK("<b>Sp. Atk:</b>")
    , SPDEF("<b>Sp. Def:</b>")
    , SPEED("<b>Speed:</b>")
{
    m_options.emplace_back(&LANGUAGE, "LANGUAGE");
    m_options.emplace_back(&HP, "HP");
    m_options.emplace_back(&ATTACK, "ATTACK");
    m_options.emplace_back(&DEFENSE, "DEFENSE");
    m_options.emplace_back(&SPATK, "SPATK");
    m_options.emplace_back(&SPDEF, "SPDEF");
    m_options.emplace_back(&SPEED, "SPEED");
}


void GenerateIVCheckerOCR::program(SingleSwitchProgramEnvironment& env){
    IVCheckerReaderScope reader(m_reader, env.console, LANGUAGE);

    QString path = "IVCheckerOCR/";
    path += QString::fromStdString(language_data(LANGUAGE).code);

    QDir dir(path);
    if (!dir.exists()){
        dir.mkpath(".");
    }
    path += "/";

    std::vector<QImage> images = reader.dump_images(env.console.video().snapshot());

    QString now = QString::fromStdString(now_to_filestring());
    images[0].save(path + QString::fromStdString(IVCheckerOptionOCR::TOKENS[HP]) + "-" + now + "a.png");
    images[1].save(path + QString::fromStdString(IVCheckerOptionOCR::TOKENS[ATTACK]) + "-" + now + "b.png");
    images[2].save(path + QString::fromStdString(IVCheckerOptionOCR::TOKENS[DEFENSE]) + "-" + now + "c.png");
    images[3].save(path + QString::fromStdString(IVCheckerOptionOCR::TOKENS[SPATK]) + "-" + now + "d.png");
    images[4].save(path + QString::fromStdString(IVCheckerOptionOCR::TOKENS[SPDEF]) + "-" + now + "e.png");
    images[5].save(path + QString::fromStdString(IVCheckerOptionOCR::TOKENS[SPEED]) + "-" + now + "f.png");

}



}
}
}

