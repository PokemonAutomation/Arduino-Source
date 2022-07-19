/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh_ShinyEncounterTester.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyEncounterTester_Descriptor::ShinyEncounterTester_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyEncounterTester",
        STRING_POKEMON + " SwSh", "Shiny Encounter Tester",
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
            "Wild Encounter",
            "Raid Den",
        },
        0
    )
{
    PA_ADD_OPTION(ENCOUNTER_TYPE);
}


void ShinyEncounterTester::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    ShinyDetectionResult result = detect_shiny_battle(
        env.console, context,
        ENCOUNTER_TYPE == 0 ? SHINY_BATTLE_REGULAR : SHINY_BATTLE_RAID,
        std::chrono::seconds(30)
    );
    result.best_screenshot->save(now_to_filestring() + ".png");
}





}
}
}
