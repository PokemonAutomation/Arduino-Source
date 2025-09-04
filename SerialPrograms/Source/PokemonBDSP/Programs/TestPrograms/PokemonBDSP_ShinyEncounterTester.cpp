/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP_ShinyEncounterTester.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShinyEncounterTester_Descriptor::ShinyEncounterTester_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:ShinyEncounterTester",
        STRING_POKEMON + " BDSP", "Shiny Encounter Tester",
        "",
        "Test the shiny encounter detector. Start this program just before an encounter.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        {}
    )
{}



ShinyEncounterTester::ShinyEncounterTester()
    : ENCOUNTER_TYPE(
        "<b>Encounter Type:</b>",
        {
            {BattleType::STARTER, "starter", "Starter Battle"},
            {BattleType::STANDARD, "standard", "Wild Encounter"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        BattleType::STANDARD
    )
    , USE_SOUND_DETECTION(
        "<b>Use Sound Detection:</b><br>Use sound to improve shiny detection.<br>"
        "<b>Make sure you have correct audio input set.</b>",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , NOTIFICATIONS({
        &NOTIFICATION_ERROR_RECOVERABLE,
    })
{
    PA_ADD_OPTION(ENCOUNTER_TYPE);
    PA_ADD_OPTION(USE_SOUND_DETECTION);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void ShinyEncounterTester::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DoublesShinyDetection result_wild;
    ShinyDetectionResult result_own;
    detect_shiny_battle(
        env, env.console, context,
        result_wild, result_own,
        NOTIFICATION_ERROR_RECOVERABLE,
        ENCOUNTER_TYPE == BattleType::STARTER ? YOUR_POKEMON : WILD_POKEMON,
        std::chrono::seconds(30),
        USE_SOUND_DETECTION
    );
    if (ENCOUNTER_TYPE == BattleType::STARTER){
        result_own.get_best_screenshot().save(now_to_filestring() + ".png");
    }else{
        result_wild.get_best_screenshot().save(now_to_filestring() + ".png");
    }
}





}
}
}
