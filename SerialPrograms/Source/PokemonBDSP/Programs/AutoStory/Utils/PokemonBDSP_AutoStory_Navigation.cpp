/*  BDSP AutoStory - Navigation Utilities
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP_AutoStory_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


void walk_steps(ProControllerContext& context, uint16_t direction, uint16_t steps){
    // TODO: implement — press D-pad `direction` for `steps` tiles
    (void)context; (void)direction; (void)steps;
}

void walk_until_dialog(
    VideoStream& stream,
    ProControllerContext& context,
    uint16_t direction,
    uint16_t timeout_sec
){
    // TODO: implement — walk in direction; stop when ShortDialogWatcher fires
    (void)stream; (void)context; (void)direction; (void)timeout_sec;
}

void walk_until_battle(
    VideoStream& stream,
    ProControllerContext& context,
    uint16_t direction,
    uint16_t timeout_sec
){
    // TODO: implement — walk in direction; stop when StartBattleDetector fires
    (void)stream; (void)context; (void)direction; (void)timeout_sec;
}

void recalibrate_after_battle(
    const ProgramInfo& /*info*/,
    VideoStream& /*stream*/,
    ProControllerContext& context,
    std::function<void(ProControllerContext&)>&& recovery_action
){
    // TODO: implement — wait for overworld, then run recovery_action
    recovery_action(context);
}


}
}
}
