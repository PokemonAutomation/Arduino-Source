/*  Pokemon RSE Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Soft reset, menus, etc.
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/UnexpectedBattleException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonRSE/PokemonRSE_Settings.h"
#include "PokemonRSE_Navigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{


void soft_reset(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context){
	// A + B + Select + Start
	pbf_press_button(context, BUTTON_B | BUTTON_Y | BUTTON_MINUS | BUTTON_PLUS, 10, 180);

	pbf_mash_button(context, BUTTON_PLUS, GameSettings::instance().START_BUTTON_MASH);
	context.wait_for_all_requests();

	pbf_press_button(context, BUTTON_A, 20, 40);

	//Wait for game to load in
	BlackScreenOverWatcher detector(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    int ret = wait_until(
        console, context,
        std::chrono::milliseconds(GameSettings::instance().ENTER_GAME_WAIT * (1000 / TICKS_PER_SECOND)),
        {{detector}}
    );
    if (ret == 0){
        console.log("Entered game!");
    }else{
        console.log("Timed out waiting to enter game.", COLOR_RED);
    }
	context.wait_for_all_requests();
}









}
}
}
