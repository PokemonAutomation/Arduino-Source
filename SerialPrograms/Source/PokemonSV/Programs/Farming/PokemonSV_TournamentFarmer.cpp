/*  Tournament Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/PokemonSV_MoneyReader.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
#include "PokemonSV/Inference/PokemonSV_TournamentPrizeNameReader.h"
#include "PokemonSV/Resources/PokemonSV_TournamentPrizeNames.h"
#include "PokemonSV_TournamentFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;

TournamentFarmer_Descriptor::TournamentFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:TournamentFarmer",
        STRING_POKEMON + " SV", "Tournament Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/TournamentFarmer.md",
        "Farm the Academy Ace Tournament for money and prizes.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct TournamentFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : tournaments(m_stats["Tournaments won"])
        , battles(m_stats["Battles fought"])
        , losses(m_stats["Losses"])
        , money(m_stats["Money made"])
        , matches(m_stats["Items matched"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Tournaments won");
        m_display_order.emplace_back("Battles fought");
        m_display_order.emplace_back("Losses");
        m_display_order.emplace_back("Money made");
        m_display_order.emplace_back("Items matched");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& tournaments;
    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& losses;
    std::atomic<uint64_t>& money;
    std::atomic<uint64_t>& matches;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> TournamentFarmer_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


TournamentFarmer::StopButton::StopButton()
    : ButtonOption(
      "<b>Stop after Current Tournament:",
      "Stop after current Tournament",
      0, 16
    )
{}
void TournamentFarmer::StopButton::set_idle(){
    this->set_enabled(false);
    this->set_text("Stop after current Tournament");
}
void TournamentFarmer::StopButton::set_ready(){
    this->set_enabled(true);
    this->set_text("Stop after current Tournament");
}
void TournamentFarmer::StopButton::set_pressed(){
    this->set_enabled(false);
    this->set_text("Program will stop after current tournament...");
}


TournamentFarmer::~TournamentFarmer(){
    STOP_AFTER_CURRENT.remove_listener(*this);
}
TournamentFarmer::TournamentFarmer()
    : NUM_ROUNDS(
        "<b>Number of Tournaments to run:",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 0
    )
    , TRY_TO_TERASTILLIZE(
        "<b>Use Terastillization:</b><br>Tera at the start of battle. Will take longer to complete each tournament but may be worth the attack boost.<br>This setting is not necessary if you are running a set specifically made to farm the tournament.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , SAVE_NUM_ROUNDS(
        "<b>Save every this many tournaments:</b><br>Zero disables saving. Will save win or lose.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1, 0
    )
    , MONEY_LIMIT(
        "<b>Stop after earning this amount of money:</b><br>Zero disables this check. Does not count losses. In-game maximum is 9,999,999. This can be set up to 999,999,999.",
        LockMode::UNLOCK_WHILE_RUNNING,
        9999999, 0, 999999999
    )
    , HHH_ZOROARK(
        "<b>Happy Hour H-Zoroark:</b><br>Check this if you have an event Hisuian Zoroark with Happy Hour and Memento as your lead.<br>Happy Hour must be in its first move slot and Memento must be in its second.<br>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b><br>The language is needed to read the prizes.",
        TournamentPrizeNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , TARGET_ITEMS("<b>Items:</b>")
    , NOTIFICATION_PRIZE_MATCH("Matching Prize", true, false, ImageAttachmentMode::JPG, { "Notifs" })
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_PRIZE_MATCH,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_stop_after_current(false)
{
    PA_ADD_OPTION(STOP_AFTER_CURRENT);
    PA_ADD_OPTION(NUM_ROUNDS);
    PA_ADD_OPTION(TRY_TO_TERASTILLIZE);
    PA_ADD_OPTION(SAVE_NUM_ROUNDS);
    PA_ADD_OPTION(MONEY_LIMIT);
    PA_ADD_OPTION(HHH_ZOROARK);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(TARGET_ITEMS);
    PA_ADD_OPTION(NOTIFICATIONS);

    STOP_AFTER_CURRENT.set_idle();
    STOP_AFTER_CURRENT.add_listener(*this);
}

void TournamentFarmer::on_press(){
    global_logger_tagged().log("Stop after current requested...");
    m_stop_after_current.store(true, std::memory_order_relaxed);
    STOP_AFTER_CURRENT.set_pressed();
}

//Check and process the amount of money earned at the end of a battle
void TournamentFarmer::check_money(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    TournamentFarmer_Descriptor::Stats& stats = env.current_stats<TournamentFarmer_Descriptor::Stats>();

    int top_money = -1;
    int bottom_money = -1;

    //There must be a value for top money
    //Bottom money only appear after 1st battle and should clear out
    for (uint16_t c = 0; c < 3 && top_money == -1; c++){
        VideoSnapshot screen = env.console.video().snapshot();
        ImageFloatBox top_notif(0.745, 0.152, 0.206, 0.083);
        ImageFloatBox bottom_notif(0.745, 0.261, 0.220, 0.083);

        ImageRGB32 image_top = to_blackwhite_rgb32_range(
            extract_box_reference(screen, top_notif),
            true,
            combine_rgb(215, 215, 215), combine_rgb(255, 255, 255)
        );
        //image_top.save("./image_top.png");

        //Different color range on the bottom notif ~B0B5B8
        ImageRGB32 image_bottom = to_blackwhite_rgb32_range(
            extract_box_reference(screen, bottom_notif),
            true,
            combine_rgb(130, 130, 130), combine_rgb(240, 240, 240)
        );
        //image_bottom.save("./image_bottom.png");
        
        top_money = OCR::read_money(env.console, image_top);
        bottom_money = OCR::read_money(env.console, image_bottom);

        //dump_image(
        //    env.console, env.program_info(),
        //    "battledone",
        //    screen
        //);

        //Filter out low and high numbers in case of misreads
        //From bulbapedia: Nemona is lowest at 8640,  Penny and Geeta are highest at 16800
        //Max earnings? in one battle: happy hour * amulet coin * (base winnings + (8 * make it rain lv100))
        if (top_money < 8000 || top_money > 80000 ){
            top_money = -1;
        }
        if (bottom_money < 8000 || bottom_money > 80000){
            bottom_money = -1;
        }
    }

    if (top_money != -1){
        //If both notification boxes appear take the newer one.
        if (bottom_money != -1){
            stats.money += bottom_money;
            env.update_stats();
        }else{
            stats.money += top_money;
            env.update_stats();
        }
    }else{
        env.log("Unable to read money.");
    }

}


//Handle a single battle by mashing A until AdvanceDialog (end of battle) is detected
void TournamentFarmer::run_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    TournamentFarmer_Descriptor::Stats& stats = env.current_stats<TournamentFarmer_Descriptor::Stats>();

    //Only applies if the player has The Hidden Treasure of Area Zero Hisuian Zoroark
    if (HHH_ZOROARK){
        env.log("Zoroark option checked.");
        
        //Use happy hour
        env.log("Using Happy Hour.");
        pbf_mash_button(context, BUTTON_A, 300);
        context.wait_for_all_requests();

        //If not already dead, use memento and die
        NormalBattleMenuWatcher memento(COLOR_RED);
        MoveSelectDetector move_select(COLOR_BLUE);
        SwapMenuWatcher fainted(COLOR_YELLOW);
        int retZ = wait_until(
            env.console, context,
            std::chrono::seconds(60),
            { memento, fainted }
        );
        if (retZ == 0){
            env.log("Using Memento to faint.");
            pbf_press_button(context, BUTTON_A, 10, 50);
            move_select.move_to_slot(env.console, context, 1);
            pbf_press_button(context, BUTTON_A, 10, 50);
            pbf_wait(context, 100);
            context.wait_for_all_requests();

            pbf_press_dpad(context, DPAD_DOWN, 10, 50);
            pbf_press_button(context, BUTTON_A, 10, 50);
            context.wait_for_all_requests();

            //Try six times, in case of paralysis (only applies to Pachirisu's Nuzzle) preventing use of Memento.
            int retF = run_until<ProControllerContext>(
                env.console, context,
                [&](ProControllerContext& context){
                    for (size_t c = 0; c < 6; c++){
                        NormalBattleMenuWatcher battle_memento(COLOR_RED);
                        int ret_memento = wait_until(
                            env.console, context,
                            std::chrono::seconds(60),
                            { battle_memento }
                        );
                        if (ret_memento == 0){
                            env.log("Attempting to use Memento.");
                            pbf_mash_button(context, BUTTON_A, 300);
                            context.wait_for_all_requests();
                        }
                    }
                },
                { fainted }
            );

            if (retF == 0){
                env.log("Swap menu detected.");
            }else{
                env.log("Took more than 6 turns to use Memento. Was Zoroark able to faint?", COLOR_RED);
                stats.errors++;
                env.update_stats();
                send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Took more than 6 turns to use Memento. Was Zoroark able to faint?",
                    env.console
                );
            }

        }else if (retZ == 1){
            env.log("Detected swap menu. Assuming Zoroark fainted turn one.");
        }else{
            env.log("Timed out after Happy Hour.", COLOR_RED);
            stats.errors++;
            env.update_stats();
            send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Timed out after Happy Hour.",
                env.console
            );
        }

        //Select 2nd pokemon from swap menu and send it out
        fainted.move_to_slot(env.console, context, 1);
        pbf_mash_button(context, BUTTON_A, 300);
        context.wait_for_all_requests();

        //Check for battle menu to ensure it's sent out
        NormalBattleMenuWatcher resume_battle(COLOR_RED);
        int retRes = wait_until(
            env.console, context,
            std::chrono::seconds(60),
            { resume_battle }
        );
        if (retRes == 0){
            env.log("Battle menu detected. Second Pokemon has been sent out. Resuming usual battle sequence.");
        }else{
            env.log("Could not find battle menu.", COLOR_RED);
            stats.errors++;
            env.update_stats();
            send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Could not find battle menu.",
                env.console
            );
        }

    }

    //Assuming the player has a charged orb
    if (TRY_TO_TERASTILLIZE){
        env.log("Attempting to terastillize.");
        //Open move menu
        pbf_press_button(context, BUTTON_A, 10, 50);
        pbf_wait(context, 100);
        context.wait_for_all_requests();

        pbf_press_button(context, BUTTON_R, 20, 50);
        pbf_press_button(context, BUTTON_A, 10, 50);
    }

    //Mash A until battle finished
    AdvanceDialogWatcher end_of_battle(COLOR_YELLOW);
    WallClock start = current_time();
    uint8_t switch_party_slot = HHH_ZOROARK ? 2: 1;
    int ret_black = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            for(size_t c = 0; c < 30; c++) { //Sylveon build has 16 PP at max, and Chi-Yu build has 24.
                if (current_time() - start > std::chrono::minutes(5)){
                    env.log("Timed out during battle after 5 minutes.", COLOR_RED);
                    stats.errors++;
                    env.update_stats();
                    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Timed out during battle after 5 minutes.",
                        env.console
                    );
                }

                GradientArrowWatcher switch_pokemon(COLOR_BLUE, GradientArrowType::RIGHT, {0.50, 0.40, 0.20, 0.30});
                NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
                MoveSelectWatcher move_select(COLOR_GREEN);
                SwapMenuWatcher fainted(COLOR_RED);
                context.wait_for_all_requests();

                int ret = wait_until(
                    env.console, context,
                    std::chrono::seconds(90), //Tera takes ~25 to 30 seconds each, slightly over 60 seconds if both player and opponent uses in the same turn
                    {
                        switch_pokemon,
                        battle_menu,
                        move_select,
                        fainted,
                    }   //End of battle from tera'd ace takes longer, 45 seconds was not enough
                );
                switch (ret){
                case 0:
                    env.log("Detected switch " + STRING_POKEMON + " prompt. Pressing B to not switch...");
                    pbf_mash_button(context, BUTTON_B, 3 * TICKS_PER_SECOND);
                    break;
                case 1:
                    env.log("Detected battle menu. Pressing A to attack...");
                    pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
                    break;
                case 2:
                    env.log("Detected move selection. Pressing A to attack...");
                    pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
                    break;
                case 3:
                    // Since we can't run from the tournament, loop through all party Pokemon spamming their first move.
                    env.log("Detected fainted " + STRING_POKEMON + ". Switching to next living " + STRING_POKEMON + "...");
                    if (fainted.move_to_slot(env.console, context, switch_party_slot)){
                        pbf_mash_button(context, BUTTON_A, 3 * TICKS_PER_SECOND);
                        switch_party_slot++;
                    }
                    break;
                default:
                    env.log("Timed out during battle. Stuck, crashed, or took more than 90 seconds for a turn.", COLOR_RED);
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Timed out during battle. Stuck, crashed, or took more than 90 seconds for a turn.",
                        env.console
                    );
                }
            }
        },
        { end_of_battle }
        );
    if (ret_black == 0){
        env.log("Battle finished."); //Cannot tell if win or loss.
        stats.battles++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        //Close dialog and then check money
        pbf_press_button(context, BUTTON_B, 10, 50);
        pbf_wait(context, 100);
        context.wait_for_all_requests();

        check_money(env, context);

        //Clear any remaining dialog
        pbf_mash_button(context, BUTTON_B, 300);
        context.wait_for_all_requests();
    }else{
        env.log("Timed out during battle. Stuck, crashed, or took over 30 turns.", COLOR_RED);
        stats.errors++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Timed out during battle. Stuck, crashed, or took over 30 turns.",
            env.console
        );
    }
    context.wait_for_all_requests();
}


//Check prize and notify if it matches filters after a tournament win
void TournamentFarmer::check_prize(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    TournamentFarmer_Descriptor::Stats& stats = env.current_stats<TournamentFarmer_Descriptor::Stats>();

    VideoSnapshot screen = env.console.video().snapshot();
    ImageFloatBox dialog_box(0.259, 0.734, 0.484, 0.158);
    ImageViewRGB32 dialog_image = extract_box_reference(screen, dialog_box);

    //bool replace_color_within_range = false;
    //ImageRGB32 dialog_filtered = filter_rgb32_range(
    //    extract_box_reference(screen, dialog_box),
    //    combine_rgb(50, 135, 162), combine_rgb(167, 244, 255), Color(0), replace_color_within_range
    //);
    //dialog_filtered.save("./dialog_image.png");

    const double LOG10P_THRESHOLD = -1.5;
    OCR::StringMatchResult result = PokemonSV::TournamentPrizeNameReader::instance().read_substring(
        env.console, LANGUAGE, dialog_image,
        OCR::BLUE_TEXT_FILTERS()
    );
    result.clear_beyond_log10p(LOG10P_THRESHOLD);
    if (result.results.empty()){
        env.log("No matching prize name found in dialog box.");
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    }
    for (const auto& r : result.results){
        env.console.log("Found prize: " + r.second.token);
        if (TARGET_ITEMS.find_item(r.second.token)){
            env.log("Prize matched");

            stats.matches++;
            env.update_stats();

            send_program_notification(
                env, NOTIFICATION_PRIZE_MATCH,
                COLOR_GREEN, "Prize matched",
                {
                    { "Item:", get_tournament_prize_name(r.second.token).display_name() },
                }
            , "", screen);
            break;
        }
    }
}


//Tournament won and over
void TournamentFarmer::handle_end_of_tournament(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    TournamentFarmer_Descriptor::Stats& stats = env.current_stats<TournamentFarmer_Descriptor::Stats>();

    //Space out the black screen detection after the "champion" battle
    //pbf_wait(context, 700);
    //context.wait_for_all_requests();

    //One more black screen when done to load the academy
    BlackScreenOverWatcher black_screen(COLOR_RED, { 0.2, 0.2, 0.6, 0.6 });
    int ret_black_won = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 10000);
        },
        { black_screen }
        );
    if (ret_black_won == 0){
        env.log("Tournament complete, waiting for academy.");
    }
    context.wait_for_all_requests();

    //Wait for congrats dialog - wait an extra bit since the dialog appears while still loading in
    AdvanceDialogWatcher advance_detector(COLOR_YELLOW);
    int ret_dialog = wait_until(env.console, context, Milliseconds(1000), { advance_detector });
    if (ret_dialog == 0){
        env.log("Dialog detected.");
    }
    pbf_wait(context, 300);
    context.wait_for_all_requests();

    //Next dialog is prize dialog - wait until all dialog has appeared then check prize
    pbf_press_button(context, BUTTON_A, 10, 100);
    pbf_wait(context, 100);
    context.wait_for_all_requests();

    check_prize(env, context);

    //Clear remaining dialog
    OverworldWatcher overworld(env.console, COLOR_CYAN);
    int ret_over = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_B, 700);
        },
        { overworld }
        );
    if (ret_over != 0){
        env.console.log("Failed to detect overworld.", COLOR_RED);
    }
    context.wait_for_all_requests();

    stats.tournaments++;
    env.update_stats();
    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
}


//Fly to academy from west pokemon center after losing.
void return_to_academy_after_loss(
    ProgramEnvironment& env,
    VideoStream& stream,
    ProControllerContext& context
){
    env.log("Tournament lost! Navigating back to academy.");
    go_to_academy_fly_point(env, stream, context);


    pbf_wait(context, 100);
    context.wait_for_all_requests();

    env.log("At academy fly point. Heading back to doors.");
    pbf_move_left_joystick(context, 0, 128, 8, 0);
    pbf_press_button(context, BUTTON_L, 50, 40);
    pbf_press_button(context, BUTTON_PLUS, 50, 40);
    pbf_press_button(context, BUTTON_B, 50, 40); //Trying to jump/glide over npc spawns
    pbf_press_button(context, BUTTON_B, 50, 40);
    pbf_move_left_joystick(context, 128, 0, 500, 0);
    pbf_press_button(context, BUTTON_B, 50, 40);
    pbf_press_button(context, BUTTON_B, 50, 40);

    BlackScreenOverWatcher black_screen(COLOR_RED, { 0.2, 0.2, 0.6, 0.6 });
    int ret_black_lost = run_until<ProControllerContext>(
        stream, context,
        [](ProControllerContext& context){
            pbf_move_left_joystick(context, 128, 0, 5000, 0);
        },
        { black_screen }
        );
    context.wait_for_all_requests();
    if (ret_black_lost == 0){
        env.log("Black screen detected.");
    }

    //Wait for academy to load.
    OverworldWatcher overworld(stream.logger(), COLOR_CYAN);
    int ret_academy = wait_until(stream, context, Milliseconds(4000), { overworld });
    if (ret_academy == 0){
        env.log("Entered academy. Walking to tournament entry.");
    }
    context.wait_for_all_requests();

    //Move to tournament entry
    pbf_move_left_joystick(context, 128, 0, 500, 0);
    pbf_move_left_joystick(context, 0, 128, 100, 0);
    pbf_move_left_joystick(context, 255, 0, 100, 0);
    context.wait_for_all_requests();
}

void go_to_academy_fly_point(ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context){
    int numAttempts = 0;
    int maxAttempts = 5;

    bool isFlySuccessful = false;

    while (!isFlySuccessful && numAttempts < maxAttempts ){
        open_map_from_overworld(env.program_info(), stream, context);
        pbf_press_button(context, BUTTON_ZR, 50, 40);
        pbf_move_left_joystick(context, 200, 0, 47, 25);  
        // pbf_move_left_joystick(context, 187, 0, 50, 0);
        numAttempts++;
        isFlySuccessful = fly_to_overworld_from_map(env.program_info(), stream, context, true);
        if (!isFlySuccessful){
            env.log("Unsuccessful fly attempt.");
        }
        pbf_mash_button(context, BUTTON_B, 100);
    }

    if(!isFlySuccessful){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to fly back to academy!",
            stream
        );
    }

}



class TournamentFarmer::ResetOnExit{
public:
    ResetOnExit(StopButton& button)
        : m_button(button)
    {}
    ~ResetOnExit(){
        m_button.set_idle();
    }

private:
    StopButton& m_button;
};



void TournamentFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    TournamentFarmer_Descriptor::Stats& stats = env.current_stats<TournamentFarmer_Descriptor::Stats>();

    m_stop_after_current.store(false, std::memory_order_relaxed);
    STOP_AFTER_CURRENT.set_ready();
    ResetOnExit reset_button_on_exit(STOP_AFTER_CURRENT);

    /*
    Preconditions:
    Last Pokemon Center visited is Mesagzoa West
    Sylveon only farming build - ideally with fairy tera
    stand in front of tournament entry man
    Ride legendary is not the solo pokemon (in case of loss)
    Do not have other notifications on screen for money reading (ex. new outbreak)

    Possible improvements to make:
    find prize sprites - some code is there, just disabled
    find translations for tera shards
    other languages: make sure "bottle cap" isn't misread as "bottle of PP Up"
    */

    for (uint32_t c = 0; c < NUM_ROUNDS; c++){
        if (m_stop_after_current.load(std::memory_order_relaxed)){
            break;
        }

        env.log("Tournament loop started.");

        //  Initiate dialog then mash until first battle starts
        AdvanceDialogWatcher advance_detector(COLOR_YELLOW);
        pbf_press_button(context, BUTTON_A, 10, 50);
        pbf_press_button(context, BUTTON_A, 10, 50);
        int ret = wait_until(env.console, context, Milliseconds(7000), { advance_detector });
        if (ret == 0){
            env.log("Dialog detected.");
        }else{
            env.log("Dialog not detected.");
        }
        pbf_mash_button(context, BUTTON_A, 400);
        context.wait_for_all_requests();

        NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
        int ret_battle = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 10000); //it takes a while to load and start
            },
            { battle_menu }
        );
        if (ret_battle != 0){
            env.console.log("Failed to detect battle start!", COLOR_RED);
        }
        context.wait_for_all_requests();

        bool battle_lost = false;
        for (uint16_t battles = 0; battles < 4; battles++){
            NormalBattleMenuWatcher battle_menu2(COLOR_YELLOW); //Next battle started
            OverworldWatcher overworld(env.console, COLOR_CYAN); //Previous battle was lost
            int ret_battle2 = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
                },
                { battle_menu2, overworld }
            );
            context.wait_for_all_requests();

            switch (ret_battle2){
            case 0:
                env.log("Detected battle menu.");
                run_battle(env, context);
                break;
            case 1:
                env.log("Detected overworld.");
                battle_lost = true;
                stats.losses++;
                env.update_stats();
                send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                break;
            default:
                env.log("Failed to detect battle menu or dialog prompt!");
                stats.errors++;
                env.update_stats();
                send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed to detect battle menu or dialog prompt!",
                    env.console
                );
                break;
            }

            //If this is the last battle in the tournament check for overworld in case player lost
            if (battles == 3){
                env.log("Final battle of the tournament complete, checking for overworld/loss.");

                //Clear dialog, mash B
                pbf_mash_button(context, BUTTON_B, 400);
                context.wait_for_all_requests();

                OverworldWatcher overworld2(env.console, COLOR_RED);
                int ret_lost_final = wait_until(
                    env.console, context,
                    std::chrono::seconds(3),
                    { overworld2 }
                );
                switch (ret_lost_final){
                case 0:
                    env.log("Final battle of the tournament lost.");
                    battle_lost = true;
                    stats.losses++;
                    env.update_stats();
                    send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
                    break;
                default:
                    env.log("Final battle of the tournament won.");
                    break;
                }
            }

            if (battle_lost){
                return_to_academy_after_loss(env, env.console, context);
                break;
            }
        }

        //Tournament won
        if (!battle_lost){
            handle_end_of_tournament(env, context);
        }

        env.log("Tournament loop complete.");

        //Save the game if option is set
        uint16_t num_rounds_temp = SAVE_NUM_ROUNDS;
        if (num_rounds_temp != 0 && ((c + 1) % num_rounds_temp) == 0){
            env.log("Saving game.");
            save_game_from_overworld(env.program_info(), env.console, context);
        }

        //Break loop and finish program if money limit is hit
        uint32_t earnings_temp = MONEY_LIMIT;
        if (earnings_temp != 0 && stats.money >= earnings_temp){
            env.log("Money limit hit. Ending program.");
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}

}
}
}

