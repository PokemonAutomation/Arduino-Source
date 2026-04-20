/*  Read Battle Level Up
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include <string>
#include <optional>
#include "Common/Cpp/Color.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_BattleDialogs.h"
#include "PokemonFRLG/Inference/PokemonFRLG_BattleLevelUpReader.h"
#include "PokemonFRLG_ReadBattleLevelUp.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

using namespace std::chrono_literals;


ReadBattleLevelUp_Descriptor::ReadBattleLevelUp_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:ReadBattleLevelUp",
        Pokemon::STRING_POKEMON + " FRLG",
        "Read Battle Level-Up Stats", "",
        "Read stats from the relevant popup after leveling up during a battle.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
){}

ReadBattleLevelUp::ReadBattleLevelUp()
{}

void ReadBattleLevelUp::program(
    SingleSwitchProgramEnvironment &env,
    ProControllerContext &context
){
    env.log(
        "Starting Read Battle Level Up program..."
    );

    BattleLevelUpDetector plus_detector(COLOR_RED, BattleLevelUpDialog::plus);
    BattleLevelUpDetector stats_detector(COLOR_RED, BattleLevelUpDialog::stats);

    env.log("Detecting the relevant dialogue box...");
    VideoSnapshot screen = env.console.video().snapshot();
    bool plus_dialog = plus_detector.detect(screen);
    bool stats_dialog = stats_detector.detect(screen);

    if (plus_dialog){
        env.log("Wrong dialogue box is visible. Stats are read from the next one.");
    }
    if (!stats_dialog){
        env.log("Stats dialogue box not detected.");
    }else{
        env.log("Stats dialogue box detected!");
    }


    BattleLevelUpReader reader(COLOR_RED);
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);


    env.log("Reading stats...");
    VideoSnapshot screen1 = env.console.video().snapshot();
    StatReads stats = reader.read_stats(env.logger(), screen1);

    env.log("Max HP: " + (stats.hp > 0 ? std::to_string(stats.hp) : "???"));
    env.log("Attack: " +
            (stats.attack > 0 ? std::to_string(stats.attack) : "???"));
    env.log("Defense: " +
            (stats.defense > 0 ? std::to_string(stats.defense) : "???"));
    env.log("Sp. Attack: " +
            (stats.spatk > 0 ? std::to_string(stats.spatk) : "???"));
    env.log("Sp. Defense: " +
            (stats.spdef > 0 ? std::to_string(stats.spdef) : "???"));
    env.log("Speed: " +
            (stats.speed > 0 ? std::to_string(stats.speed) : "???"));

    env.log("Finished Reading Stats. Verification boxes are on overlay.",
                    COLOR_BLUE);
    pbf_wait(context, 10s);
    context.wait_for_all_requests();
}

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

