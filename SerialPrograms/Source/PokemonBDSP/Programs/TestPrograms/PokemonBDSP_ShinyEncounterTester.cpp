/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP_ShinyEncounterTester.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxGenderDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShinyEncounterTester_Descriptor::ShinyEncounterTester_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonBDSP:ShinyEncounterTester",
        STRING_POKEMON + " BDSP", "Shiny Encounter Tester",
        "",
        "Test the shiny encounter detector. Start this program just before an encounter.",
        FeedbackType::REQUIRED, true,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyEncounterTester::ShinyEncounterTester(const ShinyEncounterTester_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , ENCOUNTER_TYPE(
        "<b>Encounter Type:</b>",
        {
            "Starter Battle",
            "Wild Encounter",
        },
        1
    )
    , NOTIFICATIONS({
        &NOTIFICATION_ERROR_RECOVERABLE,
    })
{
    PA_ADD_OPTION(ENCOUNTER_TYPE);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void ShinyEncounterTester::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    //const char* colour[3] = { "Chikorita-M", "Chikorita-F", "Mew" };
    const char* colour[6] = { "Chikorita", "Jigglypuff", "Starly", "Chikorita-M", "Chikorita-F", "Mew" };
    QImage image;
    for (int i = 0; i < 6; i++)
    {
        QString path = "D:\\Side-Projects\\PokemonAutomation\\PythonHelper\\images\\";
            path+= colour[i];
            path+= ".png";
        image.load(path);

        QString sending = "Sending: ";
            sending+=colour[i];
            env.log(sending);

        read_gender(env.console, env.console, image, colour[i]);
    }


//    DoublesShinyDetection result_wild;
//    ShinyDetectionResult result_own;
//    detect_shiny_battle(
//        env.console, context,
//        result_wild, result_own,
//        env.program_info(), NOTIFICATION_ERROR_RECOVERABLE,
//        ENCOUNTER_TYPE == 0 ? YOUR_POKEMON : WILD_POKEMON,
//        std::chrono::seconds(30)
//    );
}





}
}
}
