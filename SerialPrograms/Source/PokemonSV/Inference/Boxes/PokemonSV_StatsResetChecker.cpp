/*  Egg Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Options/Pokemon_EggHatchFilter.h"
#include "Pokemon/Options/Pokemon_StatsResetFilter.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxEggDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_IVCheckerReader.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_SandwichRoutines.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
    namespace PokemonSV {

    bool check_stats_reset_info(
        const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
        OCR::LanguageOCROption& LANGUAGE, Pokemon::StatsResetFilterTable& FILTERS,
        Pokemon::EggHatchAction& action
    ) {
        context.wait_for_all_requests();

        change_stats_view_to_judge(info, console, context);

        VideoOverlaySet overlay_set(console.overlay());
        IVCheckerReaderScope iv_reader_scope(console.overlay(), LANGUAGE);
        VideoSnapshot screen = console.video().snapshot();
        IVCheckerReader::Results IVs = iv_reader_scope.read(console.logger(), screen);
        const bool shiny = 0;

        console.log(IVs.to_string(), COLOR_GREEN);
        action = FILTERS.get_action(shiny, IVs);

        return shiny;
    }

}
}
}
