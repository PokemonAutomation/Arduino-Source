/*  Generate IV Checker OCR Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QDir>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh_GenerateIVCheckerOCR.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"

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
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:GenerateIVCheckerOCR",
        STRING_POKEMON + " SwSh", "Generate IV Checker OCR Data",
        "",
        "Generate IV Checker OCR Data",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}



GenerateIVCheckerOCR::GenerateIVCheckerOCR()
    : LANGUAGE(
        "<b>Game Language:</b>",
        IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING
    )
    , HP("<b>HP:</b>", IvJudgeValue::Best)
    , ATTACK("<b>Attack:</b>", IvJudgeValue::Best)
    , DEFENSE("<b>Defense:</b>", IvJudgeValue::Best)
    , SPATK("<b>Sp. Atk:</b>", IvJudgeValue::Best)
    , SPDEF("<b>Sp. Def:</b>", IvJudgeValue::Best)
    , SPEED("<b>Speed:</b>", IvJudgeValue::Best)
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(HP);
    PA_ADD_OPTION(ATTACK);
    PA_ADD_OPTION(DEFENSE);
    PA_ADD_OPTION(SPATK);
    PA_ADD_OPTION(SPDEF);
    PA_ADD_OPTION(SPEED);
}


void GenerateIVCheckerOCR::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    IvJudgeReaderScope reader(env.console, LANGUAGE);

    std::string path = "IVCheckerOCR/";
    path += language_data(LANGUAGE).code;

    QDir dir(QString::fromStdString(path));
    if (!dir.exists()){
        dir.mkpath(".");
    }
    path += "/";

    VideoSnapshot screen = env.console.video().snapshot();
    std::vector<ImageViewRGB32> images = reader.dump_images(screen);

    std::string now = now_to_filestring();
    
    const EnumDropdownDatabase<IvJudgeValue>& database = IvJudgeValue_Database();
    images[0].save(path + database.find(HP)->display + "-" + now + "a.png");
    images[1].save(path + database.find(ATTACK)->display + "-" + now + "b.png");
    images[2].save(path + database.find(DEFENSE)->display + "-" + now + "c.png");
    images[3].save(path + database.find(SPATK)->display + "-" + now + "d.png");
    images[4].save(path + database.find(SPDEF)->display + "-" + now + "e.png");
    images[5].save(path + database.find(SPEED)->display + "-" + now + "f.png");

}



}
}
}

