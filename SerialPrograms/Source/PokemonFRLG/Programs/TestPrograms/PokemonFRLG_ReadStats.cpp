/*  Read Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonFRLG_ReadStats.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG/Inference/PokemonFRLG_StatsReader.h"
#include <chrono>
#include <string>

using namespace std::chrono_literals;
namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {

ReadStats_Descriptor::ReadStats_Descriptor()
    : SingleSwitchProgramDescriptor(
          "PokemonFRLG:ReadStats", Pokemon::STRING_POKEMON + " FRLG",
          "Read Summary Stats", "",
          "Read stats, level, name, and nature from the summary screen. Start "
          "on page 1 of summary.",
          ProgramControllerClass::StandardController_NoRestrictions,
          FeedbackType::REQUIRED, AllowCommandsWhenRunning::DISABLE_COMMANDS) {}

ReadStats::ReadStats()
    : LANGUAGE("<b>Game Language:</b>",
               Pokemon::PokemonNameReader::instance().languages(),
               LockMode::LOCK_WHILE_RUNNING, true) {
  PA_ADD_OPTION(LANGUAGE);
}

void ReadStats::program(SingleSwitchProgramEnvironment &env,
                        ProControllerContext &context) {
  env.log("Starting Read Stats program... Please ensure you are on Page 1 "
          "(POKEMON INFO).");

  StatsReader reader;
  VideoOverlaySet overlays(env.console.overlay());
  reader.make_overlays(overlays);

  // Wait for user to verify boxes and show start page
  pbf_wait(context, 1000ms);
  context.wait_for_all_requests();

  PokemonFRLG_Stats stats;

  env.log("Reading Page 1 (Name, Level, Nature)...");
  VideoSnapshot screen1 = env.console.video().snapshot();
  reader.read_page1(env.logger(), LANGUAGE, screen1, stats);

  env.log("Name: " + stats.name);
  env.log("Level: " +
          (stats.level == -1 ? "???" : std::to_string(stats.level)));
  env.log("Nature: " + stats.nature);

  env.log("Navigating to Page 2 (POKEMON SKILLS)...");
  pbf_press_dpad(context, DPAD_RIGHT, 100ms, 100ms);
  context.wait_for_all_requests();
  pbf_wait(context, 500ms); // Wait for transition
  context.wait_for_all_requests();

  env.log("Reading Page 2 (Stats)...");
  VideoSnapshot screen2 = env.console.video().snapshot();
  reader.read_page2(env.logger(), screen2, stats);

  env.log("HP (Total): " + (stats.hp == -1 ? "???" : std::to_string(stats.hp)));
  env.log("Attack: " +
          (stats.attack == -1 ? "???" : std::to_string(stats.attack)));
  env.log("Defense: " +
          (stats.defense == -1 ? "???" : std::to_string(stats.defense)));
  env.log("Sp. Attack: " +
          (stats.sp_attack == -1 ? "???" : std::to_string(stats.sp_attack)));
  env.log("Sp. Defense: " +
          (stats.sp_defense == -1 ? "???" : std::to_string(stats.sp_defense)));
  env.log("Speed: " +
          (stats.speed == -1 ? "???" : std::to_string(stats.speed)));

  env.log("Finished Reading Stats. Verification boxes are on overlay.",
          COLOR_BLUE);
  pbf_wait(context, 10s);
  context.wait_for_all_requests();
}

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation
