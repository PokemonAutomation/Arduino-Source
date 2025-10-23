/*  Restaurant Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
//#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
//#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA_RestaurantFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;


RestaurantFarmer_Descriptor::RestaurantFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:RestaurantFarmer",
        STRING_POKEMON + " LZA", "Restaurant Farmer",
        "Programs/PokemonLZA/RestaurantFarmer.html",
        "Farm the restaurants for exp, items, and money.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}
class RestaurantFarmer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : battles(m_stats["Battles"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Battles");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> RestaurantFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


RestaurantFarmer::RestaurantFarmer(){}


void RestaurantFarmer::run_lobby(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    RestaurantFarmer_Descriptor::Stats& stats = env.current_stats<RestaurantFarmer_Descriptor::Stats>();

    while (true){
        context.wait_for_all_requests();

        ButtonWatcher buttonA(
            COLOR_RED,
            ButtonType::ButtonA,
            {0.1, 0.1, 0.8, 0.8},
            &env.console.overlay()
        );
        SelectionArrowWatcher arrow(
            COLOR_YELLOW, &env.console.overlay(),
            SelectionArrowType::RIGHT,
            {0.654308, 0.481553, 0.295529, 0.312621}
        );
        FlatWhiteDialogWatcher dialog0(COLOR_RED, &env.console.overlay());
        BlueDialogWatcher dialog1(COLOR_RED, &env.console.overlay());
        ItemReceiveWatcher item_receive(COLOR_RED, &env.console.overlay());

        int ret = wait_until(
            env.console, context,
            10000ms,
            {
                buttonA,
                arrow,
                dialog0,
                dialog1,
                item_receive,
            }
        );
        context.wait_for(100ms);

        switch (ret){
        case 0:
            env.log("Detected A button.");
            pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            continue;

        case 1:
            env.log("Detected selection arrow.");
            pbf_mash_button(context, BUTTON_A, 5000ms);
            return;

        case 2:
            env.log("Detected white dialog.");
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            continue;

        case 3:
            env.log("Detected blue dialog.");
            pbf_press_button(context, BUTTON_B, 160ms, 80ms);
            continue;

        case 4:
            env.log("Detected item receive.");
            pbf_press_button(context, BUTTON_A, 160ms, 80ms);
            continue;

        default:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Battle took longer than 30 minutes.",
                env.console
            );
        }
    }
}
void RestaurantFarmer::run_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    RestaurantFarmer_Descriptor::Stats& stats = env.current_stats<RestaurantFarmer_Descriptor::Stats>();

    WallClock start = current_time();


    SelectionArrowWatcher arrow(
        COLOR_YELLOW, &env.console.overlay(),
        SelectionArrowType::RIGHT,
        {0.654308, 0.481553, 0.295529, 0.312621}
    );
    ItemReceiveWatcher item_receive(COLOR_RED, &env.console.overlay(), 1000ms);
    BlueDialogWatcher dialog1(COLOR_RED, &env.console.overlay());


    int ret = run_until<ProControllerContext>(
        env.console, context,
        [=](ProControllerContext& context){
            while (current_time() - start < 30min){
                ssf_press_button(context, BUTTON_ZL, 160ms, 800ms, 200ms);
                ssf_press_button(context, BUTTON_PLUS, 320ms, 840ms);
                pbf_wait(context, 104ms);
                pbf_press_button(context, BUTTON_X, 80ms, 24ms);
                pbf_press_button(context, BUTTON_Y, 80ms, 24ms);
                pbf_press_button(context, BUTTON_B, 80ms, 24ms);
            }
        },
        {
            arrow,
            item_receive,
            dialog1,
        }
    );

    switch (ret){
    case 0:
        env.log("Detected selection arrow. (unexpected)", COLOR_RED);
        stats.errors++;
        stats.battles++;
        env.update_stats();
        return;

    case 1:
    case 2:
        env.log("Detected blue dialog. End of battle!");
        stats.battles++;
        env.update_stats();
        return;

    default:
        stats.errors++;
        env.update_stats();
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Battle took longer than 30 minutes.",
            env.console
        );
    }
}



void RestaurantFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    pbf_mash_button(context, BUTTON_B, 1000ms);

//    auto lobby = env.console.video().snapshot();

    while (true){
        run_lobby(env, context);
        run_battle(env, context);
    }


}



}
}
}
