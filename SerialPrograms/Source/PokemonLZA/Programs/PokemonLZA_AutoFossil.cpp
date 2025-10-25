/*  Auto Fossil
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA_AutoFossil.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;

// event sequence:
// A button on top of researcher Reg when in overworld 
// flat white dialog box(?) with name Reg x 3
// flat white dialog box(?) with name Reg + fossil selection menu, green right arrow
// flat white dialog box(?) with name Reg x 2
// flash of white screen
// flat white dialog box (?) with name Reg x 1
// teal dialog of "You received <fossil pokemon>"
// flat white dialog box (?) with name Reg x 2
// Back to overworld, A button shown


AutoFossil_Descriptor::AutoFossil_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:AutoFossil",
        STRING_POKEMON + " LZA", "Auto Fossil",
        "Programs/PokemonLZA/AutoFossil.html",
        "Automatically revive fossils.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}

class AutoFossil_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : fossils(m_stats["Fossils"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Fossils");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& fossils;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> AutoFossil_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


AutoFossil::AutoFossil(){}


void AutoFossil::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // AutoFossil_Descriptor::Stats& stats = env.current_stats<AutoFossil_Descriptor::Stats>();

    // TODO: Implement fossil revival logic here

    env.log("AutoFossil program started.");

    // Example loop structure
    while (true){
        context.wait_for_all_requests();

        // TODO: Add your program logic here

        pbf_wait(context, 1000ms);
    }
}



}
}
}
