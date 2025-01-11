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
	pbf_wait(context, GameSettings::instance().ENTER_GAME_WAIT);
	context.wait_for_all_requests();
}









}
}
}
