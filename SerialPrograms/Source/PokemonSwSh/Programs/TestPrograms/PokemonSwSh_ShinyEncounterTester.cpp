/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "ClientSource/Connection/BotBase.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh_ShinyEncounterTester.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyEncounterTester_Descriptor::ShinyEncounterTester_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyEncounterTester",
        STRING_POKEMON + " SwSh", "Shiny Encounter Tester",
        "",
        "Test the shiny encounter detector. Start this program just before an encounter.",
        FeedbackType::REQUIRED, AllowCommandsWhenRunning::ENABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyEncounterTester::ShinyEncounterTester()
    : ENCOUNTER_TYPE(
        "<b>Encounter Type:</b>",
        {
            {EncounterType::Wild, "wild", "Wild Encounter"},
            {EncounterType::Raid, "raid", "Raid Den"},
        },
        LockWhileRunning::LOCKED,
        EncounterType::Wild
    )
{
    PA_ADD_OPTION(ENCOUNTER_TYPE);
}


void ShinyEncounterTester::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    ShinyDetectionResult result = detect_shiny_battle(
        env.console, context,
        ENCOUNTER_TYPE == EncounterType::Wild ? SHINY_BATTLE_REGULAR : SHINY_BATTLE_RAID,
        std::chrono::seconds(30)
    );
    result.get_best_screenshot().save(now_to_filestring() + ".png");
}





}
}
}
