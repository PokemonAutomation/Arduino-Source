/*  RNG Helper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_BlindNavigation.h"
#include "PokemonFRLG_RngNavigation.h"
#include "PokemonFRLG_HardReset.h"
#include "PokemonFRLG_RngNavigation.h"
#include "PokemonFRLG_RngHelper.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

RngHelper_Descriptor::RngHelper_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:RngHelper",
        Pokemon::STRING_POKEMON + " FRLG", "RNG Helper",
        "Programs/PokemonFRLG/RngHelper.html",
        "Soft reset with specific timings for hitting a target Seed and Frame for RNG manipulation.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct RngHelper_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> RngHelper_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

RngHelper::RngHelper()
    : TARGET(
        "<b>Target:</b>",
        {
            {PokemonFRLG_RngTarget::starters, "starters", "Bulbasaur / Squirtle / Charmander"},
            {PokemonFRLG_RngTarget::magikarp, "magikarp", "Magikarp"},
            {PokemonFRLG_RngTarget::hitmon, "hitmon", "Hitmonlee / Hitmonchan"},
            {PokemonFRLG_RngTarget::eevee, "eevee", "Eevee"},
            {PokemonFRLG_RngTarget::lapras, "lapras", "Lapras"},
            {PokemonFRLG_RngTarget::fossils, "fossils", "Omanyte / Kabuto / Aerodactyl"},
            {PokemonFRLG_RngTarget::gamecornerabra, "gamecornerabra", "Game Corner Abra"},
            {PokemonFRLG_RngTarget::gamecornerclefairy, "gamecornerclefairy", "Game Corner Clefairy"},
            {PokemonFRLG_RngTarget::gamecornerdratini, "gamecornerdratini", "Game Corner Dratini"},
            {PokemonFRLG_RngTarget::gamecornerbug, "gamecornerbug", "Game Corner Bug (Scyther / Pinsir)"},
            {PokemonFRLG_RngTarget::gamecornerporygon, "gamecornerporygon", "Game Corner Porygon"},
            {PokemonFRLG_RngTarget::togepi, "togepi", "Togepi"},
            {PokemonFRLG_RngTarget::staticencounter, "staticencounter", "Static Overworld Encounters"},
            {PokemonFRLG_RngTarget::snorlax, "snorlax", "Snorlax"},
            {PokemonFRLG_RngTarget::mewtwo, "mewtwo", "Mewtwo"},
            {PokemonFRLG_RngTarget::hooh, "hooh", "Ho-oh"},
            {PokemonFRLG_RngTarget::hypno, "berryforesthypno", "Berry Forest Hypno"},
            {PokemonFRLG_RngTarget::sweetscent, "sweetscent", "Sweet Scent"},
            {PokemonFRLG_RngTarget::rocksmash, "rocksmash", "Rock Smash"},
            {PokemonFRLG_RngTarget::fishing, "fishing", "Fishing"},
            {PokemonFRLG_RngTarget::safarizonecenter, "safarizonecenter", "Safari Zone Center (Sweet Scent)"},
            {PokemonFRLG_RngTarget::safarizoneeast, "safarizoneeast", "Safari Zone East (Sweet Scent)"},
            {PokemonFRLG_RngTarget::safarizonenorth, "safarizonenorth", "Safari Zone North (Sweet Scent)"},
            {PokemonFRLG_RngTarget::safarizonewest, "safarizonewest", "Safari Zone West (Sweet Scent)"},
            {PokemonFRLG_RngTarget::safarizonesurf, "safarizonesurf", "Safari Zone Surfing"},
            {PokemonFRLG_RngTarget::safarizonefish, "safarizonefish", "Safari Zone Fishing"},
            // {PokemonFRLG_RngTarget::roaming, "roaming", "Roaming Legendaries"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        PokemonFRLG_RngTarget::starters
    )    
    , NUM_RESETS(
        "<b>Max Resets:</b><br>"
        "This program requires manual calibration, so this should usually be set to 1 while calibrating.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1, 0 // default, min
    )
    , SEED_BUTTON(
        "<b>Seed Button:</b><br>"
        "The button to be pressed on the title screen to set the seed.",
        {
            {SeedButton::A, "A", "A"},
            {SeedButton::Start, "Start", "Start"},
            {SeedButton::L, "L", "L (L=A)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        SeedButton::A
    )
    , EXTRA_BUTTON(
        "<b>Extra Button:</b><br>"
        "Additional button presses that affect the seed.",
        {
            {BlackoutButton::None, "None", "None"},
            {BlackoutButton::L, "L", "Blackout L"},
            {BlackoutButton::R, "R", "Blackout R"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        BlackoutButton::None
    )
    , SEED_DELAY(
        "<b>Seed Delay Time (ms):</b><br>"
        "The delay between starting the game and advancing past the title screen. Set this to match your target seed.",
        LockMode::LOCK_WHILE_RUNNING,
        35000, 30400 // default, min
    )
    , SEED_CALIBRATION(
         "<b>Seed Calibration (ms):</b>"
         "<br>Modifies the seed delay time. This should be changed in the opposite of the direction that you missed your seed.<br>"
         "<i>Example: if you missed your target seed by +16ms (meaning the button press was too late), <b>decrease</b> your seed calibration by -16 (shortening the delay).</i>",
        LockMode::UNLOCK_WHILE_RUNNING,
        0  // default
    )
    , CONTINUE_SCREEN_FRAMES(
        "<b>Continue Screen Frames:</b>"
        "<br>The number of RNG advances before loading the game.<br>"
        "These pass at the \"normal\" rate compared to other consoles.",
        LockMode::LOCK_WHILE_RUNNING,
        1000, 192 // default, min
    )
    , CONTINUE_SCREEN_CALIBRATION(
        "<b>Continue Screen Frames Calibration:</b>"
        "<br>A \"fine adjustment\" that modifies the RNG advances passed on the Continue Screen.<br>"
        "<i>Example: if your target advance was 10000 and you hit 10025, you can <b>decrease</b> your calibration value by 25.</i>",
        LockMode::UNLOCK_WHILE_RUNNING,
        0 // default
    )
    , INGAME_ADVANCES(
        "<b>In-Game Advances:</b>"
        "<br>The number of in-game RNG advances before triggering the gift/encounter.<br>"
        "These pass at double the rate compared to other consoles, where every frame results in 2 advances.<br>"
        "<i>Warning: this needs to be long enough to accomodate all in-game button presses prior to the gift/encounter</i>",
        LockMode::LOCK_WHILE_RUNNING,
        12345, 480 // default, min
    )
    , INGAME_CALIBRATION(
        "<b>In-Game Advances Calibration:</b>"
        "<br>A \"coarse adjustment\" that modifies the RNG advances passed after loading the game.<br>"
        "<i>Example: if your target advance was 10000 and you hit 8500, you can <b>increase</b> your calibration value by 1500.</i>",
        LockMode::UNLOCK_WHILE_RUNNING,
        0 // default
    )
    , USE_TEACHY_TV(
        "<b>Use Teachy TV:</b>"
        "<br>Opens the Teachy TV to quickly advance the RNG at 313x speed.<br>"
        "<i>Warning: can result in larger misses.</i>",
        LockMode::LOCK_WHILE_RUNNING,
        false // default
    )
    , PROFILE(
        "<b>User Profile Position:</b><br>"
        "The position, from left to right, of the Switch profile with the FRLG save you'd like to use.<br>"
        "If this is set to 0, Switch 1 defaults to the last-used profile, while Switch 2 defaults to the first profile (position 1)",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 8 // default, min, max
    )
    , TAKE_VIDEO(
        "<b>Take Video:</b><br>Record a video when the shiny is found.", 
        LockMode::LOCK_WHILE_RUNNING, 
        true // default
    )
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_SHINY(
        "Shiny found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(NUM_RESETS);
    PA_ADD_OPTION(SEED_BUTTON);
    PA_ADD_OPTION(EXTRA_BUTTON);
    PA_ADD_OPTION(SEED_DELAY);
    PA_ADD_OPTION(SEED_CALIBRATION);
    PA_ADD_OPTION(CONTINUE_SCREEN_FRAMES);
    PA_ADD_OPTION(CONTINUE_SCREEN_CALIBRATION);
    PA_ADD_OPTION(INGAME_ADVANCES);
    PA_ADD_OPTION(INGAME_CALIBRATION);
    PA_ADD_OPTION(USE_TEACHY_TV);
    PA_ADD_OPTION(PROFILE);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void RngHelper::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    RngHelper_Descriptor::Stats& stats = env.current_stats<RngHelper_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    bool shiny_found = false;

    double FRAMERATE = 59.999977; // FPS
    double FRAME_DURATION = 1000 / FRAMERATE;

    const int64_t FIXED_SEED_OFFSET = -845; // milliseconds. approximate

    while (!shiny_found){
        // prepare timings
        uint64_t TOTAL_SEED_DELAY = SEED_DELAY + SEED_CALIBRATION + FIXED_SEED_OFFSET;

        double MODIFIED_INGAME_ADVANCES = INGAME_ADVANCES + INGAME_CALIBRATION;
        if (MODIFIED_INGAME_ADVANCES < 0) {
           OperationFailedException::fire(
                ErrorReport::NO_ERROR_REPORT,
                "In-game advances cannot be negative. Check your in-game advances and calibration.",
                env.console
            ); 
        }
        uint64_t TEACHY_ADVANCES = 0;

        const bool SAFARI_ZONE = (TARGET == PokemonFRLG_RngTarget::safarizonecenter
            || TARGET == PokemonFRLG_RngTarget::safarizoneeast
            || TARGET == PokemonFRLG_RngTarget::safarizonenorth
            || TARGET == PokemonFRLG_RngTarget::safarizonewest
            || TARGET == PokemonFRLG_RngTarget::safarizonesurf
            || TARGET == PokemonFRLG_RngTarget::safarizonefish 
        );

        uint64_t TEACHY_TV_BUFFER = SAFARI_ZONE ? 12000 : 5000; // Safari zone targets need extra time to walk to the right position

        bool should_use_teachy_tv = USE_TEACHY_TV && (TARGET != PokemonFRLG_RngTarget::starters) && (MODIFIED_INGAME_ADVANCES > TEACHY_TV_BUFFER); // don't use Teachy TV for short in-game advance targets
        if (should_use_teachy_tv) {
            TEACHY_ADVANCES = uint64_t((int)std::floor((MODIFIED_INGAME_ADVANCES - TEACHY_TV_BUFFER) / 313) * 313);
        }

        const uint64_t CONTINUE_SCREEN_DELAY = uint64_t((CONTINUE_SCREEN_FRAMES + CONTINUE_SCREEN_CALIBRATION) * FRAME_DURATION);
        const uint64_t TEACHY_DELAY = uint64_t(TEACHY_ADVANCES * FRAME_DURATION / 313);
        const uint64_t INGAME_DELAY = uint64_t((MODIFIED_INGAME_ADVANCES - TEACHY_ADVANCES) * FRAME_DURATION / 2) - (should_use_teachy_tv ? 13700 : 0);
        env.log("Continue Screen delay: " + std::to_string(CONTINUE_SCREEN_DELAY) + "ms");
        env.log("In-game delay: " + std::to_string(INGAME_DELAY) + "ms");
        env.log("Teachy TV delay: " + std::to_string(TEACHY_DELAY) + "ms");
        env.log("Total time: " + std::to_string(TOTAL_SEED_DELAY + CONTINUE_SCREEN_DELAY + INGAME_DELAY + TEACHY_DELAY) + "ms");

        check_timings(env.console, TARGET, TOTAL_SEED_DELAY, CONTINUE_SCREEN_DELAY, INGAME_DELAY, SAFARI_ZONE);

        
        // handle the blind part
        reset_and_perform_blind_sequence(
            env.console, context, TARGET, 
            SEED_BUTTON, EXTRA_BUTTON, TOTAL_SEED_DELAY, 
            CONTINUE_SCREEN_DELAY, TEACHY_DELAY, INGAME_DELAY, 
            SAFARI_ZONE, PROFILE
        );
        env.log("Blind button presses complete.");
        stats.resets++;

        // detect shinies
        shiny_found = check_for_shiny(env.console, context, TARGET);
        if (shiny_found){
            env.log("Shiny found!");
            stats.shinies++;
            send_program_notification(
                env,
                NOTIFICATION_SHINY,
                COLOR_YELLOW,
                "Shiny found!",
                {}, "",
                env.console.video().snapshot(),
                true
            );
            if (TAKE_VIDEO){
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
            }
            break;
        }else if (stats.resets >= NUM_RESETS){
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Maximum resets reached."
            );
            break;
        }else{
            env.log("Pokemon is not shiny.");
            env.log("Resetting.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Resetting."
            );
            env.update_stats();
            context.wait_for_all_requests();
        }
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

