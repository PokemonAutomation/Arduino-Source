/*  Watt Trader Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/PokemonSwSh_DialogTriangleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh_DateSpam-WattTraderFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


WattTraderFarmer_Descriptor::WattTraderFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:WattTraderFarmer",
        STRING_POKEMON + " SwSh", "Date Spam - Watt Trader Farmer",
        "",
        "Buy as much stuff from a watt trader as possible - day skipping as needed to reroll items.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

class WattTraderFarmer_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : day_skips(m_stats["Day Skips"])
        , items_probed(m_stats["Items Probed"])
        , failed(m_stats["Failed"])
        , success(m_stats["Success"])
        , errors(m_stats["Errors"])
    {
        m_display_order.insert(m_display_order.begin() + 0, Stat("Day Skips"));
        m_display_order.insert(m_display_order.begin() + 1, Stat("Items Probed"));
        m_display_order.insert(m_display_order.begin() + 2, Stat("Failed"));
        m_display_order.insert(m_display_order.begin() + 3, Stat("Success"));
        m_display_order.insert(m_display_order.begin() + 4, Stat("Errors"));
    }

public:
    std::atomic<uint64_t>& day_skips;
    std::atomic<uint64_t>& items_probed;
    std::atomic<uint64_t>& failed;
    std::atomic<uint64_t>& success;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> WattTraderFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



WattTraderFarmer::WattTraderFarmer(){

}



class BuyCursorDetector : public SelectionArrowFinder{
public:
    BuyCursorDetector(VideoOverlay& overlay)
        : SelectionArrowFinder(overlay, ImageFloatBox(0.448775, 0.087129, 0.062361, 0.596040))
    {}
};
class BuyQuantityDetector : public VisualInferenceCallback{
public:
    BuyQuantityDetector()
        : VisualInferenceCallback("BuyQuantityDetector")
        , m_box(0.814031, 0.681188, 0.170379, 0.045545)
    {}
    virtual void make_overlays(VideoOverlaySet& items) const override{
        items.add(COLOR_GREEN, m_box);
    }
    virtual bool process_frame(const VideoSnapshot& frame) override{
        ImageStats stats = image_stats(extract_box_reference(frame, m_box));
        return is_solid(stats, {0.62963, 0.37037, 0.});
    }

private:
    ImageFloatBox m_box;
};



void WattTraderFarmer::buy_one(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    WattTraderFarmer_Descriptor::Stats& stats = env.current_stats<WattTraderFarmer_Descriptor::Stats>();

    bool purchased = false;
    while (true){
        DialogTriangleDetector dialog(env.logger(), env.console, true);
        BuyCursorDetector item_select(env.console);
        BuyQuantityDetector quantity;
        context.wait_for_all_requests();
        int ret = wait_until(
            env.console, context,
            std::chrono::seconds(10),
            {
                dialog,
                item_select,
                quantity,
            }
        );
        context.wait_for(200ms);
        switch (ret){
        case 0:
            env.log("Detected dialog...", COLOR_BLUE);
            if (!purchased){
                stats.failed++;
                env.update_stats();
            }
            pbf_press_button(context, BUTTON_B, 200ms, 800ms);
            return;
        case 1:
            env.log("Detected item select...", COLOR_BLUE);
            if (purchased){
                return;
            }
            pbf_press_button(context, BUTTON_A, 200ms, 800ms);
            continue;
        case 2:
            env.log("Detected quantity select...", COLOR_BLUE);
            stats.success++;
            env.update_stats();
            pbf_press_dpad(context, DPAD_DOWN, 200ms, 800ms);
            pbf_press_button(context, BUTTON_A, 200ms, 800ms);
            pbf_press_button(context, BUTTON_A, 200ms, 800ms);
            purchased = true;
            continue;
        default:
            env.log("No recognized state after 5 seconds.", COLOR_RED);
            stats.errors++;
            env.update_stats();
            pbf_mash_button(context, BUTTON_B, 5000ms);
            pbf_press_button(context, BUTTON_A, 200ms, 800ms);
            pbf_press_button(context, BUTTON_A, 200ms, 800ms);
            continue;
        }
    }
}



void WattTraderFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    WattTraderFarmer_Descriptor::Stats& stats = env.current_stats<WattTraderFarmer_Descriptor::Stats>();

    uint8_t year = MAX_YEAR;
    while (true){
        pbf_press_button(context, BUTTON_A, 200ms, 800ms);
        pbf_press_button(context, BUTTON_A, 200ms, 800ms);

        for (size_t c = 0; c < 7; c++){
            buy_one(env, context);
            stats.items_probed++;
            env.update_stats();
            pbf_press_dpad(context, DPAD_DOWN, 200ms, 200ms);
        }

        pbf_mash_button(context, BUTTON_B, 5000ms);

        //  Tap HOME and quickly spam B. The B spamming ensures that we don't
        //  accidentally update the system if the system update window pops up.
        ssf_press_button(context, BUTTON_HOME, 120ms, 160ms);
        pbf_mash_button(context, BUTTON_B, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0.get() - 120ms);

        home_roll_date_enter_game_autorollback(env.console, context, year);
        pbf_mash_button(context, BUTTON_B, 90);
        stats.day_skips++;
            env.update_stats();
    }
}





}
}
}
