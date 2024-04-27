/*  Material Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <atomic>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoHpReader.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IvJudgeReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_LetsGoTools.h"
#include "PokemonSV_MaterialFarmer.h"


// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



MaterialFarmer::MaterialFarmer()
{
    PA_ADD_OPTION(MATERIAL_FARMER_OPTIONS);
}


void MaterialFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    


    assert_16_9_720p_min(env.logger(), env.console);

    //  Connect the controller.
    pbf_press_button(context, BUTTON_L, 10, 50);

    // Throw user setup errors early in program
    // - Ensure language is set
    const Language language = MATERIAL_FARMER_OPTIONS.LANGUAGE;
    if (language == Language::None) {
        throw UserSetupError(env.console.logger(), "Must set game language option to read ingredient lists.");
    }

    // - Ensure audio input is enabled
    LetsGoKillSoundDetector audio_detector(env.console, [](float){ return true; });
    wait_until(
        env.console, context,
        std::chrono::milliseconds(1100),
        {
            static_cast<AudioInferenceCallback&>(audio_detector)
        }
    );
    audio_detector.throw_if_no_sound(std::chrono::milliseconds(1000));

    // start by warping to pokecenter for positioning reasons
    if (!MATERIAL_FARMER_OPTIONS.SKIP_WARP_TO_POKECENTER){
        reset_to_pokecenter(env.program_info(), env.console, context);
    }

    MaterialFarmer_Descriptor::Stats& stats = env.current_stats<MaterialFarmer_Descriptor::Stats>();
    run_material_farmer(env, context, MATERIAL_FARMER_OPTIONS, stats);
    
}




}
}
}
