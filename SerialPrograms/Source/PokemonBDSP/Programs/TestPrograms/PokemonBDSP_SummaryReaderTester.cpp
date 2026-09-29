/*  Summary Reader Tester
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_NatureChecker.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/Inference/Rng/PokemonBDSP_SummaryReader.h"
#include "PokemonBDSP_SummaryReaderTester.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace std::chrono_literals;
using namespace Pokemon;


SummaryReaderTester_Descriptor::SummaryReaderTester_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonBDSP:SummaryReaderTester",
        STRING_POKEMON + " BDSP", "Summary Reader Tester",
        "",
        "Read a " + STRING_POKEMON + "'s nature, gender and stats off its summary pages. "
        "Start on the Trainer Memo page.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}


SummaryReaderTester::SummaryReaderTester()
    : LANGUAGE(
        "<b>Game Language:</b><br>Needed for the nature, which is read as text.",
        summary_nature_languages(),
        LockMode::LOCK_WHILE_RUNNING, true
    )
{
    PA_ADD_OPTION(LANGUAGE);
}


static std::string or_unread(int16_t value){
    return value < 0 ? "???" : std::to_string(value);
}


void SummaryReaderTester::program(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context
){
    env.log("Start on the Trainer Memo page — the one reading "
        "\"This " + STRING_POKEMON + " is pretty [nature] by nature.\"");
    env.log("If no boxes appear on the video, check that the overlay's box display "
        "is switched on for this console.");

    VideoOverlaySet overlays(env.console.overlay());
    SummaryReader reader;

    {
        reader.make_memo_overlays(overlays);

        VideoSnapshot screen = env.console.video().snapshot();
        NatureCheckerValue nature = reader.read_nature(env.logger(), LANGUAGE, screen);
        BdspGender gender = reader.read_gender(env.logger(), screen);

        env.log("Nature: " + std::string(
            nature == NatureCheckerValue::UnableToDetect
                ? "??? (not read)"
                : NATURE_CHECKER_VALUE_STRINGS().get_string(nature)
        ), COLOR_BLUE);
        env.log("Gender: " + std::string(bdsp_gender_name(gender)), COLOR_BLUE);
        if (gender == BdspGender::Genderless){
            env.log("Genderless means the symbol was not found. A starter always has one.",
                COLOR_ORANGE);
        }

        pbf_wait(context, 5s);
        context.wait_for_all_requests();
    }

    env.log("Moving to the " + STRING_POKEMON + " Skills page...");
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
    context.wait_for_all_requests();
    pbf_wait(context, 1500ms);
    context.wait_for_all_requests();

    {
        overlays.clear();
        reader.make_skills_overlays(overlays);

        VideoSnapshot screen = env.console.video().snapshot();
        StatReads stats = reader.read_stats(env.logger(), screen);
        BdspGender gender = reader.read_gender(env.logger(), screen);

        env.log("Gender (from this page): " + std::string(bdsp_gender_name(gender)), COLOR_BLUE);
        env.log("HP (total): " + or_unread(stats.hp), COLOR_BLUE);
        env.log("Attack:     " + or_unread(stats.attack), COLOR_BLUE);
        env.log("Defense:    " + or_unread(stats.defense), COLOR_BLUE);
        env.log("Sp. Atk:    " + or_unread(stats.spatk), COLOR_BLUE);
        env.log("Sp. Def:    " + or_unread(stats.spdef), COLOR_BLUE);
        env.log("Speed:      " + or_unread(stats.speed), COLOR_BLUE);


        size_t unread = 0;
        for (int16_t value : {stats.hp, stats.attack, stats.defense,
                              stats.spatk, stats.spdef, stats.speed}){
            if (value < 0){
                unread++;
            }
        }
        if (unread == 0){
            env.log("All six read.", COLOR_BLUE);
        }else{
            env.log(std::to_string(unread) + " of 6 stats could not be read. One is survivable; "
                "several means the boxes need moving, and the overlay will show where.",
                COLOR_ORANGE);
        }

        env.log("Boxes are on the overlay. Compare them against the numbers on screen.",
            COLOR_BLUE);
        pbf_wait(context, 15s);
        context.wait_for_all_requests();
    }
}




}
}
}
