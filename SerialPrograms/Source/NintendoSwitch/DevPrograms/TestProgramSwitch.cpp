/*  Test Program (Switch)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "TestProgramSwitch.h"

//#include <immintrin.h>
#include <QApplication>
#include <QClipboard>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "Common/Cpp/Concurrency/PeriodicScheduler.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_IvJudgeReader.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleBallReader.h"
#include "PokemonLA/Programs/PokemonLA_LeapPokemonActions.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxGenderDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapZoomLevelReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ShinySymbolDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DetectHome.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogArrowDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip.h"
#include "NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_KeyboardCodeEntry.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/Battles/PokemonSV_BasicCatcher.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/Inference/Battles/PokemonSV_PostCatchDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_BattleBallReader.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV/Programs/Trading/PokemonSV_TradeRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "PokemonSV/Programs/Boxes/PokemonSV_BoxRoutines.h"
#include "PokemonSV/Programs/Eggs/PokemonSV_EggRoutines.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "PokemonSV/Inference/PokemonSV_PokePortalDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraRaidSearchDetector.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraRoutines.h"
#include "PokemonSV/Programs/FastCodeEntry/PokemonSV_CodeEntry.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_AreaZeroSkyDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "PokemonSV/Inference/PokemonSV_ZeroGateWarpPromptDetector.h"
#include "PokemonSV/Programs/ShinyHunting/PokemonSV_AreaZeroPlatform.h"
#include "PokemonSV/Inference/PokemonSV_SweatBubbleDetector.h"
#include "PokemonSV/Programs/PokemonSV_AreaZero.h"
#include "PokemonSV/Programs/Battles/PokemonSV_Battles.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Battles/PokemonSV_TeraBattleMenus.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichIngredientDetector.h"
#include "PokemonSV/Programs/Sandwiches/PokemonSV_IngredientSession.h"
#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleBallReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSV/Programs/PokemonSV_ConnectToInternet.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameSave.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "PokemonSV/Inference/PokemonSV_BagDetector.h"
#include <filesystem>
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h"
#include "PokemonSV/Inference/PokemonSV_StatHexagonReader.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "PokemonSwSh/Inference/PokemonSwSh_ReceivePokemonDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h"
#include "PokemonSV/Programs/Battles/PokemonSV_SinglesBattler.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterPrizeReader.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterJobsDetector.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterMaterialDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_DirectionDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IvJudgeReader.h"
//#include "CommonFramework/Environment/SystemSleep.h"
#include "CommonFramework/ErrorReports/ErrorReports.h"
#include "PokemonLA/Inference/Map/PokemonLA_OutbreakReader.h"
#include "PokemonSV/Programs/Farming/PokemonSV_AuctionFarmer.h"
#include "PokemonLA/Inference/Objects/PokemonLA_MMOQuestionMarkDetector.h"
//#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonSV/Programs/Farming/PokemonSV_MaterialFarmerTools.h"
#include "PokemonSV/Programs/Farming/PokemonSV_TournamentFarmer.h"
#include "NintendoSwitch/Programs/FastCodeEntry/NintendoSwitch_NumberCodeEntry.h"
#include "PokemonSV/Inference/ItemPrinter/PokemonSV_ItemPrinterMenuDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichHandDetector.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "CommonTools/Images/ImageFilter.h"
#include "NintendoSwitch/Options/NintendoSwitch_ModelType.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch/Inference/NintendoSwitch_ConsoleTypeDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_StartGameUserSelectDetector.h"
#include "NintendoSwitch/Inference/NintendoSwitch_UpdatePopupDetector.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_RollDateForward1.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip_US.h"
#include "NintendoSwitch/Programs/DateManip/NintendoSwitch_DateManip_24h.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "NintendoSwitch/Inference/NintendoSwitch2_BinarySliderDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSV/Inference/Battles/PokemonSV_StartBattleYellowBar.h"
#include "PokemonLA/Inference/Map/PokemonLA_SelectedRegionDetector.h"
#include "PokemonHome/Inference/PokemonHome_BallReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSide.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathMap.h"
#include "NintendoSwitch/Inference/NintendoSwitch_SelectedSettingDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_DialogBoxDetector.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "PokemonLGPE/Inference/Battles/PokemonLGPE_BattleArrowDetector.h"


#include <QPixmap>
#include <QVideoFrame>

//#include <Windows.h>
#include <iostream>
using std::cout;
using std::endl;


using namespace PokemonAutomation::Kernels;
using namespace PokemonAutomation::Kernels::Waterfill;



namespace PokemonAutomation{
namespace NintendoSwitch{






TestProgram_Descriptor::TestProgram_Descriptor()
    : MultiSwitchProgramDescriptor(
        "NintendoSwitch:TestProgram",
        "Nintendo Switch", "Test Program (Switch)",
        "",
        "Test Program (Switch)",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::OPTIONAL_,
        AllowCommandsWhenRunning::ENABLE_COMMANDS,
        1, 4, 1
    )
{}


TestProgram::~TestProgram(){
    BUTTON1.remove_listener(*this);
    BUTTON0.remove_listener(*this);
}
TestProgram::TestProgram()
    : BUTTON0(
        "Button Text 0", 0, 16
    )
    , BUTTON1(
        "<b>Button Label:</b><br><font color=\"red\">asdfasdfasdf</font>",
        "Button Text 1", 0, 16
    )
    , LANGUAGE(
        "<b>OCR Language:</b>",
        PokemonSwSh::IV_READER().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , IMAGE_PATH(false, "Path to image for testing", LockMode::UNLOCK_WHILE_RUNNING, "default.png", "default.png")
    , STATIC_TEXT("Test text...")
    , BOX("Box", LockMode::UNLOCK_WHILE_RUNNING, 0, 0, 1, 1)
    , NOTIFICATION_TEST("Test", true, true, ImageAttachmentMode::JPG)
    , NOTIFICATIONS({
        &NOTIFICATION_TEST,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(BUTTON0);
    PA_ADD_OPTION(BUTTON1);
    PA_ADD_OPTION(LANGUAGE);
//    PA_ADD_OPTION(CONSOLE_MODEL);
    PA_ADD_OPTION(IMAGE_PATH);
    PA_ADD_OPTION(STATIC_TEXT);
    PA_ADD_OPTION(BOX);
//    PA_ADD_OPTION(battle_AI);
    PA_ADD_OPTION(NOTIFICATIONS);
    BUTTON0.add_listener(*this);
    BUTTON1.add_listener(*this);
}


//using namespace Kernels;
using namespace Kernels::Waterfill;

using namespace PokemonSV;


void TestProgram::on_press(){
    global_logger_tagged().log("Button Pressed");
//    BUTTON.set_enabled(false);
    BUTTON0.set_text("Button Pressed");
    BUTTON1.set_text("Button Pressed");
}









void TestProgram::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    using namespace Kernels;
    using namespace Kernels::Waterfill;
    using namespace OCR;
    using namespace NintendoSwitch;
    using namespace Pokemon;
    using namespace PokemonSwSh;
//    using namespace PokemonBDSP;
    using namespace PokemonLA;
//    using namespace PokemonSV;

    [[maybe_unused]] Logger& logger = env.logger();
    [[maybe_unused]] ConsoleHandle& console = env.consoles[0];
//    [[maybe_unused]] BotBase& botbase = env.consoles[0];
    [[maybe_unused]] VideoFeed& feed = env.consoles[0];
    [[maybe_unused]] VideoOverlay& overlay = env.consoles[0];
    ProControllerContext context(scope, console.controller<ProController>());
    VideoOverlaySet overlays(overlay);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if 1
    // context.controller().monitor_keyboard_events();
#endif


#if 0

    HomeMenuDetector detector(console);
    detector.make_overlays(overlays);
#endif


#if 0
//    auto screenshot = feed.snapshot();

    PokemonLGPE::BattleArrowWatcher detector(COLOR_RED, {0.004251, 0.638941, 0.062699, 0.115312});
    detector.make_overlays(overlays);

    while (true){
        cout << detector.process_frame(feed.snapshot()) << endl;
        scope.wait_for(100ms);
    }
#endif



#if 0
    AsyncCommandSession<ProController> session(
        scope, logger, env.realtime_dispatcher(),
        console.controller<ProController>()
    );

    Milliseconds delay = 500ms;

    for (int c = 0; c < 100; c++){
        session.dispatch([](ProControllerContext& context){
            pbf_mash_button(context, BUTTON_A, 360000ms);
        });

        delay = 552ms;

        cout << "delay = " << delay.count() << endl;

        context.wait_for(delay);

        session.stop_command();

        pbf_press_button(context, BUTTON_B, 5000ms, 800ms);
        context.wait_for_all_requests();

        delay += 1ms;
    }
#endif




#if 0
    auto screenshot = feed.snapshot();



    PokemonSV::OverworldDetector detector;
    detector.make_overlays(overlays);
    cout << detector.detect(screenshot) << endl;
#endif



#if 0
    TeraCardReader detector;
    detector.make_overlays(overlays);

    cout << detector.detect(screenshot) << endl;
    detector.pokemon_slug(logger, env.program_info(), screenshot);
#endif

#if 0
    NewsDetector detector;
    detector.make_overlays(overlays);
    cout << detector.detect(screenshot) << endl;
#endif

#if 0
    PokemonSwSh::BlackDialogBoxDetector detector(true);
    detector.make_overlays(overlays);
    cout << detector.process_frame(screenshot, current_time()) << endl;
#endif


//    PokemonSV::BoxShinyDetector detector;
//    cout << detector.detect(screenshot) << endl;




#if 0
    YCommIconDetector detector(true);
    cout << detector.process_frame(screenshot, current_time()) << endl;
#endif

#if 0
    bool switch2 = true;
    ImageFloatBox key1_box; 
    ImageFloatBox other_setting1; 
    ImageFloatBox other_setting2; 

    if (!switch2){
        key1_box = {0.037322, 0.451172, 0.009879, 0.113281};
        other_setting1 = {0.01, 0.451172, 0.009879, 0.113281};
        other_setting2 = {0.02, 0.451172, 0.009879, 0.113281};
    }else if (switch2){
        key1_box = {0.062706, 0.510763, 0.009901, 0.097847};
        other_setting1 = {0.02, 0.510763, 0.009901, 0.097847};
        other_setting2 = {0.04, 0.510763, 0.009901, 0.097847};            
    }     
    
    SelectedSettingWatcher key1_selected(key1_box, other_setting1, other_setting2, other_setting1);
    int ret = wait_until(
        console, context,
        Milliseconds(5000),
        {key1_selected}
    );
    if (ret < 0){  // failed to detect Key 1 being highlighted. Reset game and re-try
        console.log("claim_mystery_gift: Failed to detect the Mystery Gift window. Reset game and re-try.", COLOR_YELLOW);
        reset_game(env.program_info(), console, context);
    }   

#endif

#if 0
    auto screenshot = feed.snapshot();

    OcrFailureWatchdog watchdog(logger);
    MaxLairInternal::BattleMenuReader reader(overlay, Language::ChineseTraditional, watchdog);
    reader.read_opponent_in_summary(logger, screenshot);
#endif

#if 0
    auto screenshot = feed.snapshot();

    std::deque<OverlayBoxScope> hits;
    Pokemon::PokemonType type[4];

    bool ret = MaxLairInternal::read_type_array(
        console,
        screenshot,
        ImageFloatBox{0.150, 0.020, 0.800, 0.780},
        hits,
        4, type, nullptr
    );

    cout << "ret = " << ret << endl;
#endif

#if 0
    ItemPrinterMaterialDetector detector(COLOR_RED, LANGUAGE);
    // detector.make_overlays(overlays);
    // cout << (int)detector.find_happiny_dust_row_index(console, context) << endl;

    ImageRGB32 image(IMAGE_PATH);
    // auto image = feed.snapshot();

    for (int i = 0; i < 10; i++){
        cout << detector.detect_material_name(console, image, context, (int8_t)i) << endl;
        cout << detector.detect_material_quantity(console, image, context, (int8_t)i) << endl;
    }
#endif

#if 0
    auto screenshot = feed.snapshot();


    DateReader reader(console);
    reader.make_overlays(overlays);
    reader.read_date(logger, screenshot);
#endif

#if 0
    BinarySliderDetector detector(COLOR_BLUE, {0.836431, 0.097521, 0.069703, 0.796694});
    auto result = detector.detect(screenshot);

    for (auto& item : result){
        cout << item.first << " : " << item.second.center_y() << endl;
    }
#endif

//    LetsGoKillWatcher menu(logger, COLOR_RED, true, {0.23, 0.23, 0.04, 0.20});
//    cout << menu.detect(screenshot) << endl;



//    PokemonSwSh::MaxLairInternal::PathSelectDetector detector;
//    detector.detect(screenshot);

//    int ret = PokemonSwSh::MaxLairInternal::read_side(screenshot);
//    cout << "ret = " << ret << endl;




#if 0
    ImageRGB32 image("20250717-121112090631.png");

    AdvanceDialogDetector detector;
    detector.make_overlays(overlays);
    cout << detector.detect(image) << endl;
#endif

#if 0
    ImageRGB32 image(RESOURCE_PATH() + "PokemonHome/PokeballSprites.png");
    image = remove_white_border(image);
    image.save("test.png");
#endif


#if 0
    auto screenshot = feed.snapshot();

    PokemonHome::BallReader reader(console);

    reader.read_ball(screenshot);
#endif



//    pbf_press_button(context, );




//    PokemonLA::detect_selected_region(console, scope);




#if 0
    auto screenshot = feed.snapshot();


    StartBattleYellowBarDetector detector(COLOR_RED);
    cout << detector.detect(screenshot) << endl;
#endif



//    cout << "asdf" << endl;
//    cout << (int)settings_detect_console_type(console, context) << endl;



//    DateReader_Switch2_US reader(COLOR_RED);
//    reader.read_date(logger, screenshot);

#if 0
    HomeMenuDetector detector0(console);
//    StartGameUserSelectDetector detector1(console);
//    UpdatePopupDetector detector2(console);
    detector0.make_overlays(overlays);
//    detector1.make_overlays(overlays);
//    detector2.make_overlays(overlays);
    cout << detector0.detect(feed.snapshot()) << endl;
//    cout << detector1.detect(feed.snapshot()) << endl;
//    cout << detector2.detect(feed.snapshot()) << endl;
#endif



//    ImageRGB32 image0("menu-light.png");
//    ImageRGB32 image1("menu-dark.png");
//    ImageRGB32 image2("menu-jpn.png");

#if 0
    env.log("Touching date to prevent rollover.");
    pbf_press_button(context, BUTTON_HOME, 160ms, PokemonSwSh::GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0);
    touch_date_from_home(console, context, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
    resume_game_no_interact(console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
#endif



#if 0
    auto screenshot = feed.snapshot();


    BinarySliderDetector detector(COLOR_RED, {0.836431, 0.097521, 0.069703, 0.796694});
    auto sliders = detector.detect(screenshot);

    for (auto& item : sliders){
        cout << item.first << " : " << item.second.min_y << endl;
    }
#endif

#if 0
    ImageFloatBox box(0.842007, 0.626446, 0.050186, 0.049587);
    ImageViewRGB32 cropped = extract_box_reference(screenshot, box);

    cropped.save("temp.png");

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        cropped, 0xffc0c0c0, 0xffffffff
    );

    cout << matrix.dump() << endl;

    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 200);
    cout << "objects.size() = " << objects.size() << endl;
    for (auto& item : objects){
        extract_box_reference(cropped, item).save("test.png");
    }
#endif



//    FastCodeEntry::numberpad_enter_code(console, context, "708538991006", true);



#if 0
//    ssf_issue_scroll(context, DPAD_LEFT, 48ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_A, 96ms, 48ms, 24ms);
    ssf_press_button(context, BUTTON_L, 0ms, 48ms, 24ms);
    ssf_issue_scroll(context, DPAD_LEFT, 48ms, 48ms, 24ms);
#endif

#if 0
    DateReader reader;
    reader.make_overlays(overlays);
    auto date = reader.read_date(logger, feed.snapshot());

    cout << "date format = " << (int)date.first << endl;
//    cout << "date  = " << (int)date.second << endl;
#endif



#if 0
//    DateReader_Switch2_US reader(COLOR_RED);
    DateReader_Switch2_JP reader(COLOR_RED);
    reader.make_overlays(overlays);
    DateTime date = reader.read_date(logger, feed.snapshot());

    cout << "Month = " << (int)date.month << endl;
    cout << "Day = " << (int)date.day << endl;
    cout << "Year = " << (int)date.year << endl;
    cout << "Hour = " << (int)date.hour << endl;
    cout << "Minute = " << (int)date.minute << endl;
#endif


#if 0
    DateChangeDetector_Switch2 detector(COLOR_RED);
    detector.make_overlays(overlays);
    cout << detector.detect(feed.snapshot()) << endl;
#endif


#if 0
    DateReader reader(console);
    reader.make_overlays(overlays);
    DateTime date = reader.read_date(logger, feed.snapshot()).second;

    cout << "Month = " << (int)date.month << endl;
    cout << "Day = " << (int)date.day << endl;
    cout << "Year = " << (int)date.year << endl;
    cout << "Hour = " << (int)date.hour << endl;
    cout << "Minute = " << (int)date.minute << endl;

    while (true){
        reader.set_date(env.program_info(), console, context, DATE0);
        for (int c = 0; c < 7; c++){
            ssf_issue_scroll_ptv(context, DPAD_LEFT);
        }
        reader.set_date(env.program_info(), console, context, DATE1);
        for (int c = 0; c < 7; c++){
            ssf_issue_scroll_ptv(context, DPAD_LEFT);
        }
    }
#endif



//    rollback_hours_from_home(console, context, 3, 500ms);



#if 0
    while (true){
        roll_date_backward_N_Switch2_wired(context, 60);
        for (size_t c = 0; c < 60; c++){
            roll_date_forward_1(console, context, true);
        }
    }
#endif



#if 0
    while (true){
        home_to_date_time(console, context, true);
//        home_to_date_time_Switch2_wired_blind(context, true);
        ssf_do_nothing(context, 1000ms);
        pbf_press_button(context, BUTTON_HOME, 200ms, 1800ms);
    }
#endif




#if 0
    ConsoleTypeDetector_Home detector(console);
    detector.make_overlays(overlays);

    cout << (int)detector.detect(feed.snapshot()) << endl;
#endif



#if 0
    UpdatePopupDetector detector;
    detector.make_overlays(overlays);

    cout << detector.detect(feed.snapshot()) << endl;
#endif





#if 0
    home_to_date_time(console, context, false);
#endif

#if 0
//    std::terminate();
    ImageRGB32 image("20250503-121259857603.png");

    image = filter_rgb32_brightness(image, COLOR_RED, false, 0x00ffff01, 0, 200);
    image.save("temp.png");
#endif


#if 0
    ImageRGB32 image("20250503-121259857603.png");

    {
        TeraTypeReader reader;
        ImageMatch::ImageMatchResult results = reader.read(image);
        results.log(logger, 120);
    }
    {
        TeraSilhouetteReader reader;
        ImageMatch::ImageMatchResult results = reader.read(image);
        results.log(logger, 120);
    }
#endif

#if 0
    Milliseconds unit = 24ms;

    ssf_issue_scroll(context, DPAD_DOWN, 2*unit, 2*unit, unit);
    ssf_issue_scroll(context, DPAD_LEFT, unit, 2*unit, unit);
    ssf_issue_scroll(context, DPAD_LEFT, unit, 2*unit, unit);
    ssf_issue_scroll(context, DPAD_LEFT, unit, 2*unit, unit);


#endif



#if 0
    ssf_press_button(context, Button::BUTTON_ZR, 1s, 60h, 0ms);
//    context->issue_gyro_rotate_x(&scope, 0s, 60h, 0ms, 0x1000);
//    context->issue_gyro_rotate_y(&scope, 0s, 60h, 0ms, 0x0000);
//    context->issue_gyro_rotate_z(&scope, 0s, 60h, 0ms, 0x1000);

//    auto duration = 10s;

//    context->issue_gyro_rotate_x(&scope, duration, duration, 0s, 0x1000);
//    context->issue_nop(&scope, 60h);

#if 0
    auto duration = 15ms;
    for (size_t c = 0; c < 65536; c += 1){
        context->issue_gyro_accel_x(&scope, 0s, duration, 0s, (uint16_t)(688 + 0*c % 2));
        context->issue_gyro_accel_y(&scope, 0s, duration, 0s, (uint16_t)(1*c % 2));
        context->issue_gyro_accel_z(&scope, 0s, duration, 0s, (uint16_t)(-4038 + 0*c % 2));
        context->issue_gyro_rotate_x(&scope, 0s, duration, 0s, (uint16_t)(0x0000 + 1*c));
        context->issue_gyro_rotate_y(&scope, 0s, duration, 0s, (uint16_t)(0x0000 + 1*c));
        context->issue_gyro_rotate_z(&scope, 0s, duration, 0s, (uint16_t)(0x0000 + 0*c));
        context->issue_nop(&scope, duration);
    }
#endif
#endif


#if 0
    ImageRGB32 image("20250420-043111395281.png");

    OcrFailureWatchdog watchdog(logger);

    PokemonSwSh::MaxLairInternal::PokemonSwapMenuReader reader(logger, overlay, Language::Korean, watchdog);

    double hp[4];
    reader.read_hp(image, hp);
#endif

#if 0
    ImageRGB32 image("20250404-154507236508.png");

    ArcPhoneDetector phone(logger, overlay, std::chrono::milliseconds(250), true);

    while (true){
        cout << phone.process_frame(image, current_time()) << endl;
        scope.wait_for(100ms);
    }

#if 0
    wait_until(
        console, context,
        10000ms,
        {phone}
    );
#endif
#endif

#if 0
    // ImageRGB32 image(IMAGE_PATH);
    auto image = feed.snapshot();
#if 0
    ImageRGB32 image(IMAGE_PATH);
    // auto image = feed.snapshot();

    SandwichHandLocator hand(SandwichHandType::FREE, {0, 0, 1, 1});
    std::pair<double, double> location = hand.locate_sandwich_hand(image, {0,0,1,1});
    cout << location.first << ", " << location.second << endl;
#endif

#if 0
    ImageRGB32 image(IMAGE_PATH);
    // auto image = feed.snapshot();

    ItemPrinterMaterialDetector detector(COLOR_RED, Language::English);

    std::vector<ImageFloatBox> boxes = {
        // {0.485,0.176758,0.037,0.05}, {0.485,0.250977,0.037,0.05}, {0.485,0.325196,0.037,0.05}, {0.485,0.399415,0.037,0.05}, {0.485,0.473634,0.037,0.05}, {0.485,0.547853,0.037,0.05}, {0.485,0.622072,0.037,0.05}, {0.485,0.696291,0.037,0.05}, {0.485,0.77051,0.037,0.05}, {0.485,0.844729,0.037,0.05},
        {0.39,0.176758,0.025,0.05}, {0.39,0.250977,0.025,0.05}, {0.39,0.325196,0.025,0.05}, {0.39,0.399415,0.025,0.05}, {0.39,0.473634,0.025,0.05}, {0.39,0.547853,0.025,0.05}, {0.39,0.622072,0.025,0.05}, {0.39,0.696291,0.025,0.05}, {0.39,0.77051,0.025,0.05}, {0.39,0.844729,0.025,0.05},
    };
    for (ImageFloatBox box : boxes){
        detector.read_number(console.logger(), env.inference_dispatcher(), image, box);
    }

#endif
#endif

#if 0

    ImageRGB32 image("20250323-011605651979.png");

    DialogBoxDetector detector;
    detector.make_overlays(overlays);
    cout << detector.detect(image) << endl;

#endif


#if 0
    auto image = feed.snapshot();

    ItemPrinterMenuDetector detector(COLOR_GREEN);
    cout << detector.detect(image) << endl;
#endif


//    numberpad_enter_code(logger, context, "708538991006", false);



#if 0
    for (size_t i = 0; i < 100; i++){
        for (size_t c = 0; c < 4; c++){
            ssf_issue_scroll(context, DPAD_RIGHT, 17ms);
        }
        for (size_t c = 0; c < 4; c++){
            ssf_issue_scroll(context, DPAD_LEFT, 17ms);
        }
    }
#endif



#if 0
    while (true){
        for (size_t c = 0; c < 60; c++){
            ssf_issue_scroll(context, DPAD_DOWN, 24ms);
        }
        ssf_do_nothing(context, 1000ms);
        for (size_t c = 0; c < 60; c++){
            ssf_issue_scroll(context, DPAD_UP, 24ms);
        }
        ssf_do_nothing(context, 1000ms);
    }
#endif

//    pbf_move_left_joystick(context, 38, 38, 10000, 0);


//    ssf_issue_scroll(context, DPAD_LEFT, 0);
//    ssf_press_button(context, BUTTON_A | BUTTON_L, 3);
//    ssf_press_button(context, BUTTON_L, 0);

#if 0
    numberpad_enter_code(
        logger, context,
        "708538991006",
        false
    );
#endif

#if 0
    codeboard_enter_digits(
        logger, context, KeyboardLayout::QWERTY,
        "JRH5T9",
        true,
        CodeboardDelays{
            .hold = 5 * 8ms,
            .cool = 3 * 8ms,
            .press_delay = 4 * 8ms * 1,
            .move_delay = 5 * 8ms * 1,
            .wrap_delay = 6 * 8ms * 1,
        }
    );
#endif

//    ssf_flush_pipeline(context);


//    return_to_academy_after_loss(env, console, context);




//    fly_from_paldea_to_blueberry_entrance(env.program_info(), console, context);



#if 0
    ssf_press_button(context, BUTTON_A, 0);
    ssf_do_nothing(context, 4);
    ssf_press_button(context, BUTTON_A, 0);
    ssf_do_nothing(context, 4);
    ssf_press_button(context, BUTTON_A, 0);
    ssf_do_nothing(context, 4);
    ssf_press_button(context, BUTTON_A, 0);
    ssf_do_nothing(context, 4);
#endif



//    enter_digits(context, 8, (const uint8_t*)"56685459");

#if 0
    for (int c = 0; c < 10; c++){
        scroll_to(context, 1, 9, true);
        scroll_to(context, 9, 1, true);
    }
//    pbf_wait(context, 100);
#endif


#if 0
    ImageRGB32 image("20250131-170450792229.png");

    PokemonSwSh::BattleBallReader reader(console, Language::Korean);
    reader.read_ball(image);
#endif


#if 0
    {
        TeraTypeReader reader;
        ImageMatch::ImageMatchResult results = reader.read(image);
        results.log(logger, 100);
    }
#endif


#if 0
    ImageRGB32 image("20250125-224044294692.png");
    MaxLairInternal::BattleMenuReader reader(overlay, Language::English);
    std::set<std::string> slugs = reader.read_opponent_in_summary(logger, image);

    cout << set_to_str(slugs) << endl;
#endif

//    ssf_press_button(context, BUTTON_A, 0, 1000, 0);
//    pbf_move_left_joystick(context, 0, 0, 20, 0);



#if 0
    ImageRGB32 image("20250218-003554940153.png");
//    ImageRGB32 image("raidecho1.jpg");
//    auto image = feed.snapshot();

//    MaxLairInternal::BattleMenuReader reader(overlay, Language::English);
//    reader.read_opponent_in_summary(logger, image);

    TeraCardReader reader;
    cout << reader.detect(image) << endl;
    reader.pokemon_slug(logger, env.program_info(), image);
//    cout << (int)reader.stars(logger, env.program_info(), image) << endl;
#endif





#if 0
    ImageRGB32 image("20250112-194339635973.png");

    PokemonBDSP::SelectionArrowFinder detector0(console, {0.50, 0.58, 0.40, 0.10}, COLOR_RED);
    PokemonBDSP::SelectionArrowFinder detector1(console, {0.50, 0.52, 0.40, 0.10}, COLOR_RED);

    cout << detector0.detect(image) << endl;
    cout << detector1.detect(image) << endl;
#endif



#if 0
    PokemonSwSh::MaxLairInternal::PokemonSwapMenuReader reader(console, overlay, Language::English);

    ImageRGB32 image("20241221-123730238930.png");

    double hp[4];
    reader.read_hp(image, hp);
#endif

//    reader.read_opponent_in_summary(logger, image);

//    PokemonSwSh::find_selection_arrows(image, 10);


//    LifetimeSanitizer::terminate_with_dump();

//    PokemonSV::AuctionFarmer farmer;
//    farmer.check_offers(env);
//    std::terminate();

#if 0
    ImageRGB32 image("screenshot-20241210-110029984325.png");
//    auto image = feed.snapshot();

    MMOQuestionMarkDetector question_mark_detector(logger);
    question_mark_detector.detect_MMO_on_hisui_map(image);
#endif


#if 0
    PokemonLA::OutbreakReader reader(logger, Language::English, overlay);
    reader.make_overlays(overlays);

    ImageRGB32 image("screenshot-20241124-135028529403.png");
#endif

//    reader.read(feed.snapshot());


//    PokemonLA::ButtonDetector detector(logger, PokemonLA::ButtonType::ButtonA,);

//    while (true){
//        SystemSleepController::instance().push_screen_on();
//        scope.wait_for(std::chrono::seconds(10));
//    }

//    SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);

//    console.save_stream_history("video.mp4");

#if 0
    VideoSnapshot image = feed.snapshot();
    report_error(
        &env.logger(),
        env.program_info(),
        "testtest",
        {{"title", "message"}},
        image,
        {"test.txt"}
    );
#endif

#if 0
    VideoSnapshot image = feed.snapshot();
//    ImageRGB32 image("20250108-151305644248.png");

    DateReader date_reader;
    date_reader.make_overlays(overlays);
    auto date = date_reader.read_date(logger, image);
//    auto date = date_reader.read_date(logger, std::make_shared<ImageRGB32>(std::move(image)));
    cout << "year =  " << (int)date.second.year << endl;
    cout << "month = " << (int)date.second.month << endl;
    cout << "day =   " << (int)date.second.day << endl;
    cout << "hour =  " << (int)date.second.hour << endl;
    cout << "min =   " << (int)date.second.minute << endl;
    cout << "secs =  " << (int)date.second.second << endl;
#endif

#if 0

    VideoSnapshot image = feed.snapshot();
    DirectionDetector detector;

    // ImageRGB32 image("MaterialFarmer-1.png");
    // ImageRGB32 image("dark-capture-card_1.png");
    // DirectionDetector detector(COLOR_BLUE, ImageFloatBox(0,0,1,1));

    // std::pair<double, double> north_location = detector.locate_north(image);
    // detector.current_direction(image);
    detector.change_direction(env.program_info(), console, context, 3.14);

#endif

#if 0
    ItemPrinterMaterialDetector detector(COLOR_RED, LANGUAGE);
    detector.make_overlays(overlays);
    // cout << (int)detector.find_happiny_dust_row_index(env.inference_dispatcher(), console, context) << endl;
    // cout << (int)detector.detect_material_quantity(env.inference_dispatcher(), console, context, 2) << endl;

    // test OCR for number 1 -> 999. for black text on light background.
    // increasing quantity of materials to sell.
    for (int i = 1; i < 1000; i++){
       context.wait_for_all_requests();
        if (i != (int)detector.detect_material_quantity(env.inference_dispatcher(), console, context, 2)){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                console,
                "OCR didn't match expected value."
            );
        }
        pbf_press_dpad(context, DPAD_UP, 20, 30);
    }

    // test OCR for number 1 -> 999. for white text on dark background
    // decreasing quantity of current materials by selling.
    // for (int i = 999; i > 0; i--){
    //     context.wait_for_all_requests();
    //     if (i != (int)detector.detect_material_quantity(env.inference_dispatcher(), console, context, 2)){
    //         OperationFailedException::fire(
    //             ErrorReport::SEND_ERROR_REPORT,
    //             console,
    //             "OCR didn't match expected value."
    //         );
    //     }
    //     pbf_press_button(context, BUTTON_A, 30, 150);
    //     pbf_press_button(context, BUTTON_A, 30, 150);
    //     pbf_press_button(context, BUTTON_A, 30, 150);
    //     pbf_press_button(context, BUTTON_A, 30, 150);
    // }


#endif

#if 0
    ImageRGB32 image("screenshot-20240701-165012250266.png");

    BattleBallReader reader(console, Language::English);
    cout << reader.read_quantity(image) << endl;
#endif

#if 0
    // ImageRGB32 image("screenshot-20240701-165012250266.png");

    // BattleBallReader reader(console, Language::English);
    // cout << reader.read_quantity(image) << endl;

    VideoSnapshot image = feed.snapshot();
    // IngredientSession session(env.inference_dispatcher(), console, context, Language::English, SandwichIngredientType::CONDIMENT);
    // session.read_ingredient_quantity(console, context, 8);

    SandwichIngredientReader reader(SandwichIngredientType::FILLING);
    // ImageMatch::ImageMatchResult image_result = reader.read_with_icon_matcher(image, ImageFloatBox(0.508, 0.820, 0.032, 0.057));
    for (int i = 0; i < 6; i++){
        ImageMatch::ImageMatchResult image_result = reader.read_with_icon_matcher(image, ImageFloatBox(0.508781 + 0.0468*i, 0.820, 0.032, 0.057));
        image_result.clear_beyond_spread(SandwichIngredientReader::ALPHA_SPREAD);
        image_result.log(console, SandwichIngredientReader::MAX_ALPHA);
        image_result.clear_beyond_alpha(SandwichIngredientReader::MAX_ALPHA);
    }
#endif


#if 0
    ImageRGB32 image("screenshot-20240630-183016042676.png");

    ButtonTracker tracker(ButtonType::ButtonA);
    WhiteObjectWatcher watcher(overlay, {0.55, 0.40, 0.20, 0.40}, { {tracker, false} });
    watcher.process_frame(image, current_time());
#endif

#if 0
    VideoSnapshot screen = console.video().snapshot();
    ItemPrinterJobsDetector detector(COLOR_RED);
    cout << (int)detector.detect_jobs(logger, env.inference_dispatcher(), screen) << endl;
#endif

#if 0
    ItemPrinterMaterialDetector detector(COLOR_RED, LANGUAGE);
    detector.make_overlays(overlays);
    // cout << (int)detector.find_happiny_dust_row_index(env.inference_dispatcher(), console, context) << endl;
    cout << (int)detector.detect_material_quantity(env.inference_dispatcher(), console, context, 2) << endl;
#endif

#if 0
    VideoSnapshot screen = console.video().snapshot();
    ItemPrinterJobsDetector detector(COLOR_RED);
    cout << (int)detector.detect_jobs(logger, env.inference_dispatcher(), screen) << endl;
#endif

#if 0
    VideoSnapshot screen = console.video().snapshot();

    OverworldDetector detector;
    cout << detector.detect(screen) << endl;
#endif

#if 0
    ItemPrinterJobsDetector detector(COLOR_RED, LANGUAGE);
    detector.make_overlays(overlays);

    detector.set_print_jobs(console, context, 5);
#endif


#if 0
    ItemPrinterPrizeReader reader(Language::English);
    reader.make_overlays(overlays);

    reader.read(logger, env.inference_dispatcher(), feed.snapshot());
#endif


//    SinglesAIOption battle_AI;
//    run_singles_battle(env, console, context, battle_AI, false);




//    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
//    reset_game_from_home(env.program_info(), console, context, 5 * TICKS_PER_SECOND);


#if 0
    VideoSnapshot screen = console.video().snapshot();

    PokemonSummaryDetector summary;
    cout << summary.detect(screen) << endl;
#endif


#if 0
    ImageViewRGB32 box = extract_box_reference(screen, ImageFloatBox(0.28, 0.20, 0.03, 0.055));
    ImageStats stats = image_stats(box);
    cout << stats.average << stats.stddev << endl;

    bool item_held = !is_solid(stats, {0.550405, 0.449595, 0.}, 0.20);
    cout << "item_held = " << item_held << endl;
#endif

#if 0
    run_pokemon(
        console, context,
        {
            {SinglesMoveType::Move1, false},
            {SinglesMoveType::Move2, false},
            {SinglesMoveType::Move4, true},
            {SinglesMoveType::Run, false},
        }
    );
#endif



#if 0
    auto snapshot = console.video().snapshot();

    PokemonSummaryDetector detector;
    detector.make_overlays(overlays);
    cout << detector.detect(snapshot) << endl;
#endif


#if 0
    auto snapshot = console.video().snapshot();
    ImageViewRGB32 box0 = extract_box_reference(snapshot, ImageFloatBox{0.415, 0.085, 0.035, 0.057});
    ImageViewRGB32 box1 = extract_box_reference(snapshot, ImageFloatBox{0.553, 0.085, 0.035, 0.057});

    {
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(box1, 0xff808080, 0xffffffff);
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(20);
        WaterfillObject object;
        while (iter->find_next(object, false)){
            extract_box_reference(box1, object).save("test.png");
        }
    }
    #endif



//    PokemonSwSh::ReceivePokemonDetector caught_detector(true);
//    caught_detector.process_frame();


#if 0
    start_game_from_home(
        console, context,
        true, 0, 0,
        10
    );
#endif

#if 0
//    UpdateMenuWatcher update_menu(false);
    CheckOnlineDetector update_menu(false);
    update_menu.make_overlays(overlays);

    auto snapshot = console.video().snapshot();
    cout << update_menu.detect(snapshot) << endl;
#endif

#if 0

//    ImageRGB32 image("screenshot-20231003-202430049819.png");
    auto snapshot = console.video().snapshot();

    PokemonSummaryDetector detector;
    cout << detector.detect(snapshot) << endl;
#endif

#if 0
    SummaryStatsReader reader;
    reader.make_overlays(overlays);

    auto snapshot = console.video().snapshot();

    NatureAdjustments nature = reader.read_nature(logger, snapshot);
    cout << "attack     = " << (int)nature.attack << endl;
    cout << "defense    = " << (int)nature.defense << endl;
    cout << "spatk      = " << (int)nature.spatk << endl;
    cout << "spdef      = " << (int)nature.spdef << endl;
    cout << "speed      = " << (int)nature.speed << endl;

    StatReads stats = reader.read_stats(logger, snapshot);
    cout << "hp         = " << stats.hp << endl;
    cout << "attack     = " << stats.attack << endl;
    cout << "defense    = " << stats.defense << endl;
    cout << "spatk      = " << stats.spatk << endl;
    cout << "spdef      = " << stats.spdef << endl;
    cout << "speed      = " << stats.speed << endl;

    BaseStats base_stats{113, 70, 120, 135, 65, 52};
    EVs evs{0, 0, 0, 0, 0, 0};
    IvRanges ivs = reader.calc_ivs(logger, snapshot, base_stats, evs);
    cout << "hp         = " << (int)ivs.hp.low << " - " << (int)ivs.hp.high << endl;
    cout << "attack     = " << (int)ivs.attack.low << " - " << (int)ivs.attack.high << endl;
    cout << "defense    = " << (int)ivs.defense.low << " - " << (int)ivs.defense.high << endl;
    cout << "spatk      = " << (int)ivs.spatk.low << " - " << (int)ivs.spatk.high << endl;
    cout << "spdef      = " << (int)ivs.spdef.low << " - " << (int)ivs.spdef.high << endl;
    cout << "speed      = " << (int)ivs.speed.low << " - " << (int)ivs.speed.high << endl;
#endif


#if 0
    cout << "attack: ";
    reader.read_stat_adjustment({0.874, 0.293, 0.014, 0.024}, snapshot);
    cout << "spatk: ";
    reader.read_stat_adjustment({0.770, 0.293, 0.014, 0.024}, snapshot);
    cout << "speed: ";
    reader.read_stat_adjustment({0.822, 0.452, 0.014, 0.024}, snapshot);
#endif


#if 0
    ImageFloatBox hp    (0.823, 0.244, 0.012, 0.022);
    ImageFloatBox atk   (0.875, 0.294, 0.012, 0.022);
    ImageFloatBox def   (0.875, 0.402, 0.012, 0.022);
    ImageFloatBox spatk (0.771, 0.294, 0.012, 0.022);
    ImageFloatBox spdef (0.771, 0.402, 0.012, 0.022);
    ImageFloatBox spd   (0.823, 0.453, 0.012, 0.022);

    overlays.add(COLOR_RED, hp);
    overlays.add(COLOR_RED, atk);
    overlays.add(COLOR_RED, def);
    overlays.add(COLOR_RED, spatk);
    overlays.add(COLOR_RED, spdef);
    overlays.add(COLOR_RED, spd);
#endif



#if 0
    PokemonSwSh::MaxLairInternal::LobbyJoinedDetector detector(2, false);

    auto snapshot = console.video().snapshot();
//    detector.VisualInferenceCallback::process_frame(snapshot);
    detector.joined(snapshot, snapshot.timestamp);
#endif

//    size_t errors = 0;
//    attach_item_from_bag(env.program_info(), console, context, errors);
//    attach_item_from_box(env.program_info(), console, context, 1, errors);

//    BagDetector detector;
//    detector.make_overlays(overlays);
//    auto snapshot = console.video().snapshot();
//    cout << detector.detect(snapshot) << endl;


#if 0
    TeraCatchDetector detector(COLOR_RED);
    detector.make_overlays(overlays);

    auto snapshot = console.video().snapshot();
    cout << detector.detect(snapshot) << endl;

    detector.move_to_slot(console, context, 1);
#endif

//    std::shared_ptr<const ImageRGB32> screen(new ImageRGB32("20230920-123043559137-OperationFailedException.png"));

//    IngredientSession session(env.inference_dispatcher(), console, context, Language::English, SandwichIngredientType::FILLING);
//    session.read_screen(screen);




//    pbf_press_dpad(context, DPAD_RIGHT, 160, 0);
//    pbf_press_dpad(context, DPAD_DOWN, 40, 0);




//    PokemonLA::save_game_from_overworld(env, console, context);





#if 0
    TeraCardReader reader;

    auto snapshot = console.video().snapshot();
    cout << reader.detect(snapshot) << endl;
#endif

#if 0
    NormalDialogDetector detector(logger, overlay, true);

    detector.make_overlays(overlays);

    auto snapshot = console.video().snapshot();
    cout << detector.process_frame(snapshot, snapshot.timestamp) << endl;
#endif

//    PokemonLA::open_travel_map_from_jubilife(env, console, context);






#if 0
    FrozenImageDetector detector(std::chrono::seconds(5), 10);

    int ret = wait_until(
        console, scope, std::chrono::seconds(10),
        {detector}
    );
    if (ret >= 0){
        console.log("triggered");
    }else{
        console.log("timed out");
    }
#endif

//    NewsDetector detector;
//    detector.make_overlays(overlays);



//    VideoSnapshot image = feed.snapshot();

//    BoxSelectionBoxModeWatcher watcher;
//    watcher.process_frame(image, image.timestamp);

//    connect_to_internet_from_overworld(env.program_info(), console, context);



#if 0
    MainMenuDetector detector;
    detector.make_overlays(overlays);
    detector.move_cursor(env.program_info(), console, context, MenuSide::RIGHT, 6, true);
#endif

#if 0
    ImageRGB32 image("20230226-042613391191-PathPartyReader-ReadSprites.png");

    PokemonSwSh::MaxLairInternal::GlobalState state;
    PokemonSwSh::MaxLairInternal::PathReader reader(overlay, 0);
    reader.read_sprites(logger, state, image);
#endif

//    NintendoSwitch::PokemonSwSh::BattleBallReader reader(console, Language::English);
//    reader.read_ball(image);



//    basic_catcher(console, context, Language::English, "beast-ball", true);

#if 0
    VideoSnapshot image = feed.snapshot();
    IngredientSession session(env.inference_dispatcher(), console, context, Language::English);
    session.read_current_page();
#endif



#if 0
    add_sandwich_ingredients(
        env.inference_dispatcher(), console, context, Language::English,
        {
            {"pickle", 1},
            {"cucumber", 1},
            {"tomato", 3},
        },
        {
            {"sour-herba-mystica", 1},
            {"spicy-herba-mystica", 1},
        }
    );
#endif


#if 0
    IngredientSession session(env.inference_dispatcher(), console, context, Language::English);
//    basic_catcher(console, context, Language::English, "poke-ball", true);


#if 0
    PageIngredients page = session.read_current_page();
    for (size_t c = 0; c < 10; c++){
        cout << set_to_str(page.item[c]) << endl;
    }
#endif

//    cout << "Found: " << session.move_to_ingredient({"tomato"}) << endl;
    session.add_ingredients(console, context, {
        {"pickle", 1},
        {"cucumber", 1},
        {"tomato", 3},
    });
    pbf_press_button(context, BUTTON_PLUS, 20, 230);

    pbf_press_dpad(context, DPAD_UP, 20, 105);
    session.add_ingredients(console, context, {
        {"sour-herba-mystica", 1},
        {"spicy-herba-mystica", 1},
    });
    pbf_press_button(context, BUTTON_PLUS, 20, 230);
#endif


#if 0
    auto image = feed.snapshot();

    NormalBattleMenuDetector detector(COLOR_RED);
    detector.detect_slot(image);
    detector.move_to_slot(console, context, 0);
#endif



//    save_game_from_menu_or_overworld(env.program_info(), console, context, true);



//    auto_heal_from_menu(env.program_info(), console, context, 0, true);



//    return_to_inside_zero_gate(env.program_info(), console, context);
//    inside_zero_gate_to_secret_cave_entrance(env.program_info(), console, context);


//    inside_zero_gate_to_station(env.program_info(), console, context, 1, false);
//    ssf_press_left_joystick(context, 144, 0, 0, 5 * TICKS_PER_SECOND);
//    pbf_mash_button(context, BUTTON_A, 5 * TICKS_PER_SECOND);

//    pbf_move_left_joystick(context, 96, 255, 5 * TICKS_PER_SECOND, 0);


#if 0
    auto image = feed.snapshot();

    NewsDetector detector;
    cout << detector.detect(image) << endl;
#endif



//    pbf_mash_button(context, BUTTON_ZR, 3 * TICKS_PER_SECOND);

#if 0
    auto image = feed.snapshot();
    SweatBubbleDetector detector(COLOR_RED, {0.11, 0.81, 0.06, 0.10});
    cout << detector.detect(image) << endl;
#endif


#if 0
    NavigatePlatformSettings settings;

    return_to_inside_zero_gate(env.program_info(), console, context);
    inside_zero_gate_to_platform(env.program_info(), console, context, settings);
#endif

#if 0
    auto image = feed.snapshot();

    ImageRGB32 filtered = filter_rgb32_range(image, 0xff000040, 0xff8080ff, Color(0xffff0000), true);
    filtered.save("test.png");

    using namespace Kernels::Waterfill;

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff000040, 0xff8080ff);

//    size_t min_width = screen.width() / 4;
//    size_t min_height = screen.height() / 4;

    WaterfillObject biggest;
    WaterfillObject object;

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(10000);
    while (iter->find_next(object, false)){
//        if (object.min_y != 0){
//            continue;
//        }
        if (biggest.area < object.area){
            biggest = std::move(object);
        }
    }

    cout << biggest.center_of_gravity_x() / image->width() << ", " << biggest.center_of_gravity_y() / image->height() << endl;
#endif


#if 0
//    pbf_controller_state(context, 0, DPAD_NONE, 192, 255, 116, 128, 3 * TICKS_PER_SECOND);
    pbf_move_left_joystick(context, 192, 0, 20, 105);
    pbf_press_button(context, BUTTON_L | BUTTON_PLUS, 20, 105);

    ssf_press_button(context, BUTTON_LCLICK, 0, 500);
    ssf_press_joystick(context, true, 128, 0, 125, 1250);

    //  Jump
    ssf_press_button(context, BUTTON_B, 125, 100);

    //  Fly
    ssf_press_button(context, BUTTON_B, 0, 50);

    pbf_move_left_joystick(context, 144, 0, 750, 0);
    pbf_move_left_joystick(context, 128, 0, 1000, 0);
#endif

#if 0
    ZeroGateWarpPromptDetector detector;
    auto image = feed.snapshot();
    detector.detect(image);
#endif


//    zero_gate_to_platform(env.program_info(), console, context);


#if 0
    pbf_press_button(context, BUTTON_PLUS, 20, 105);
    pbf_move_left_joystick(context, 128, 0, 625, 0);
    ssf_press_button(context, BUTTON_B, 0, 50);
    pbf_move_left_joystick(context, 128, 0, 250, 0);
    pbf_move_left_joystick(context, 160, 0, 600, 0);
    pbf_move_left_joystick(context, 128, 0, 1875, 0);
#endif


#if 0
    EncounterWatcher encounter(console, COLOR_RED);

    throw ProgramFinishedException(
        console,
        "test",
        encounter.shiny_screenshot()
    );
#endif


#if 0
    LetsGoKillWatcher watcher(logger, COLOR_RED, false);

    wait_until(
        console, scope, std::chrono::seconds(600),
        {
            watcher,
        }
    );
#endif

#if 0
    LetsGoKillDetector detector(COLOR_RED, {0.71, 0.15, 0.04, 0.30});
    detector.make_overlays(overlays);
    while (true){
        detector.detect(feed.snapshot());
        scope.wait_for(std::chrono::milliseconds(100));
    }
#endif

#if 0
//    ImageRGB32 image("Screenshots/screenshot-20230205-141319486902.png");
    ImageRGB32 image("LetsGoKill.png");

//    LetsGoKillDetector detector(COLOR_RED, {0.5, 0, 0.5, 0.5});
    LetsGoKillDetector detector(COLOR_RED, {0, 0, 1, 1});
    detector.detect(image);
#endif


#if 0
    EncounterWatcher encounter(console);
    int ret = wait_until(
        console, scope, std::chrono::seconds(600),
        {
            static_cast<VisualInferenceCallback&>(encounter),
            static_cast<AudioInferenceCallback&>(encounter),
        }
    );
    encounter.throw_if_no_sound();

    if (ret == 0){
        logger.log("Found battle menu!");
    }

    if (encounter.shiny_screenshot()){
        encounter.shiny_screenshot()->save("test.png");
    }
#endif


#if 0
    pbf_move_left_joystick(context, 128, 255, 300, 0);
    pbf_move_left_joystick(context, 128, 0, 50, 0);
    pbf_press_button(context, BUTTON_R, 20, 0);
    pbf_move_left_joystick(context, 128, 0, 350, 0);
#endif
#if 0
    size_t count = 0;
    while (true){
        NormalBattleMenuWatcher battle_menu(COLOR_RED);
        AreaZeroSkyTracker sky_tracker(overlay);
        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            console, context,
            [&](ProControllerContext& context){
                while (true){
                    switch (count++ % 2){
                    case 0:
                        run_overworld(env, console, context, sky_tracker, 0.50);
                        break;
                    case 1:
                        run_overworld(env, console, context, sky_tracker, 0.70);
                        break;
                    }
                }
            },
            {battle_menu, sky_tracker}
        );
        context.wait_for(std::chrono::milliseconds(200));
        if (ret == 0){
            console.log("Detected battle encounter.");
            pbf_press_dpad(context, DPAD_DOWN, 250, 0);
            pbf_press_button(context, BUTTON_A, 20, 105);
            pbf_mash_button(context, BUTTON_B, 1 * TICKS_PER_SECOND);
        }
    }
#endif






#if 0
    auto image = feed.snapshot();

    ImageRGB32 filtered = filter_rgb32_range(image, 0xffe0e000, 0xffffffff, Color(0xff000000), false);
    filtered.save("test.png");
#endif

//    change_view_to_stats_or_judge(console, context);
//    change_view_to_judge(console, context, Language::English);




#if 0
    auto image = feed.snapshot();

    ImageRGB32 filtered = filter_rgb32_range(image, 0xff808000, 0xffffffff, Color(0xff000000), false);
    filtered.save("test.png");
#endif

#if 0
    TeraBattleMenuDetector battle_menu(COLOR_RED);
    MoveSelectDetector move_select(COLOR_YELLOW);
    CheerSelectDetector cheer_select(COLOR_GREEN);
    battle_menu.make_overlays(overlays);
    move_select.make_overlays(overlays);
    cheer_select.make_overlays(overlays);

    auto image = feed.snapshot();
    cheer_select.detect_slot(image);
#endif


#if 0
    auto image = feed.snapshot();
    CodeEntryDetector detector;
    detector.make_overlays(overlays);
    cout << detector.detect(image) << endl;
#endif


//    enter_tera_search(env.program_info(), console, context, false);
//    open_hosting_lobby(env.program_info(), console, context, HostingMode::ONLINE_CODED);

#if 0
    auto image = feed.snapshot();
    TeraLobbyReader detector(console.logger(), env.realtime_dispatcher());
    detector.make_overlays(overlays);
    cout << detector.detect(image) << endl;
#endif

#if 0
    size_t host_index = 1;
    ConsoleHandle& host = env.consoles[host_index];
    BotBaseContext host_context(scope, host.botbase());

    env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
        if (console.index() == host_index){
            open_raid(console, context);
        }else{
            enter_tera_search(env.program_info(), console, context, true);
        }
    });
    open_hosting_lobby(env.program_info(), host, host_context, HostingMode::ONLINE_CODED);

    TeraLobbyReader lobby_reader;
    std::string code = lobby_reader.raid_code(env.logger(), env.realtime_dispatcher(), host.video().snapshot());
    std::string normalized_code;
    const char* error = normalize_code(normalized_code, code);
    if (error){
//        pbf_press_button(host_context, BUTTON_B, 20, 230);
//        pbf_press_button(host_context, BUTTON_A, 20, 230);
        OperationFailedException::fire(env.logger(), "Unable to read raid code.");
    }

    env.run_in_parallel(scope, [&](ConsoleHandle& console, ProControllerContext& context){
        if (console.index() == host_index){
            return;
        }
        enter_code(console, context, FastCodeEntrySettings(), normalized_code, false);
    });
#endif



#if 0
    TeraRaidSearchDetector detector(COLOR_YELLOW);

    auto image = feed.snapshot();
    cout << detector.detect(image) << endl;
    detector.move_cursor_to_search(env.program_info(), console, context);
#endif

#if 0
    auto image = feed.snapshot();
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff000000, 0xff808080);
    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(100);
    WaterfillObject object;

    size_t c = 0;
    while (iter->find_next(object, false)){
//        cout << "yellow = " << object.area << endl;
        extract_box_reference(image, object).save("object-" + std::to_string(c++) + ".png");
//        yellows.emplace_back(std::move(object));
    }
#endif







#if 0
    PokePortalDetector detector(COLOR_YELLOW);

    auto image = feed.snapshot();
    cout << detector.detect_location(image) << endl;

    detector.move_cursor(env.program_info(), console, context, 2);
#endif


//    size_t errors;
//    release_box(env.program_info(), console, context, errors, 1);



#if 0
//    TeraCatchWatcher catch_menu(COLOR_BLUE);
#if 0
    WhiteButtonWatcher next_button(
        COLOR_CYAN,
        WhiteButton::ButtonA,
        {0.8, 0.93, 0.2, 0.07},
        WhiteButtonWatcher::FinderType::PRESENT,
        std::chrono::seconds(1)
    );
#endif
//    AdvanceDialogWatcher advance(COLOR_YELLOW);
    PromptDialogWatcher add_to_party(COLOR_PURPLE, {0.500, 0.395, 0.400, 0.100});
    PromptDialogWatcher view_summary(COLOR_PURPLE, {0.500, 0.470, 0.400, 0.100});
    PromptDialogWatcher nickname(COLOR_GREEN, {0.500, 0.545, 0.400, 0.100});
//    PokemonSummaryWatcher summary(COLOR_MAGENTA);
    MainMenuWatcher main_menu(COLOR_BLUE);
//    OverworldWatcher overworld(COLOR_RED);

//    catch_menu.make_overlays(overlays);
//    next_button.make_overlays(overlays);
//    advance.make_overlays(overlays);
    add_to_party.make_overlays(overlays);
    view_summary.make_overlays(overlays);
    nickname.make_overlays(overlays);
//    summary.make_overlays(overlays);
    main_menu.make_overlays(overlays);
//    overworld.make_overlays(overlays);
#endif


#if 0
    OverworldWatcher overworld;
    int ret = wait_until(
        console, context, std::chrono::seconds(5),
        {overworld}
    );
    cout << ret << endl;
#endif

#if 0
    ImageRGB32 image("BadArrow.png");

    TeraBattleMenuDetector battle_menu(COLOR_RED);
    MoveSelectDetector move_select(COLOR_GREEN);
    TargetSelectDetector target_select(COLOR_CYAN);
    TeraCatchDetector tera_catch(COLOR_BLUE);
//    battle_menu.make_overlays(overlays);
//    move_select.make_overlays(overlays);
//    target_select.make_overlays(overlays);
//    tera_catch.make_overlays(overlays);
    cout << (int)battle_menu.detect_slot(image) << endl;

//    battle_menu.move_to_slot(console, context, 0);
//    move_select.move_to_slot(console, context, 1);
//    target_select.move_to_slot(console, context, 2);
#endif


//    change_stats_view_to_judge(env.program_info(), console, context);


#if 0
    auto image = feed.snapshot();
    ImageFloatBox box(0.66, 0.08, 0.52, 0.04);
    ImageStats stats = image_stats(extract_box_reference(image, box));
    cout << stats.average << stats.stddev << endl;
#endif



#if 0
    auto image = feed.snapshot();

    TeraLobbyReader detector;
//    cout << detector.seconds_left(env.logger(), image) << endl;
    cout << detector.raid_code(env.logger(), image) << endl;
#endif



#if 0
    QClipboard* clipboard = QApplication::clipboard();
    cout << clipboard->supportsSelection() << endl;

    while (true){
        std::string code = clipboard->text(QClipboard::Selection).toStdString();
        cout << code << endl;
        scope.wait_for(std::chrono::milliseconds(1000));
    }
#endif


//    while (true){

//    }

#if 0
    pbf_move_left_joystick(context, 0, 128, 40, 40);
    // Move forward to pass table
    pbf_move_left_joystick(context, 128, 0, 80, 40); // old value: 80
    // Move right
    pbf_move_left_joystick(context, 255, 128, 40, 40);
    // Move back to face basket
    pbf_move_left_joystick(context, 128, 255, 10, 40);
#endif


#if 0
//    ImageFloatBox box(0.02, );
    auto image = feed.snapshot();
    WhiteButtonDetector detector(COLOR_RED, WhiteButton::ButtonA, {0.020, 0.590, 0.035, 0.060});
    cout << detector.detect(image) << endl;
#endif


//    auto image = feed.snapshot();
//    TeraCatchDetector detector(COLOR_RED);
//    cout << detector.detect(image) << endl;

//    size_t errors;
//    release_one_pokemon(env.program_info(), console, context, errors);


#if 0
    auto image = feed.snapshot();

    TeraLobbyReader detector;
    detector.make_overlays(overlays);
    detector.ready_players(image);
#endif

#if 0
    MainMenuDetector detector;
    detector.move_cursor(env.program_info(), console, context, MenuSide::RIGHT, 1);
    cout << "done" << endl;
#endif


#if 0
    size_t eggs_collected = 0;
    check_basket_to_collect_eggs(
        env.program_info(), console, context,
        1, eggs_collected
    );
#endif


#if 0
    auto image = feed.snapshot();
    DialogBoxDetector detector;
    cout << detector.detect(image) << endl;
#endif

#if 0
    BattleMenuDetector detector(COLOR_RED);
    while (true){
        scope.wait_for(std::chrono::milliseconds(50));
        auto image = feed.snapshot();
        cout << detector.detect(image) << endl;
    }
#endif


#if 0
    auto image = feed.snapshot();
    TeraCatchWatcher detector(COLOR_RED);
    bool ok = detector.detect(image);
    cout << ok << endl;
    if (!ok){
        image.frame->save("tmp.png");
    }

#if 0
    int ret = wait_until(
        console, context, std::chrono::seconds(60),
        {detector}
    );
    cout << "ret = " << ret << endl;
#endif
#endif


#if 0
    auto image = feed.snapshot();

    BoxDetector detector;
    cout << detector.detect(image) << endl;

    while (true){
        scope.wait_for(std::chrono::milliseconds(50));
        image = feed.snapshot();
        std::pair<BoxCursorLocation, BoxCursorCoordinates> location = detector.detect_location(image);
        cout << (int)location.first << ": " << (int)location.second.row << "," << (int)location.second.col << endl;
    }
#endif


#if 0
    GradientArrowDetector party_select_top(COLOR_GREEN, GradientArrowType::RIGHT, {0.30, 0.27, 0.10, 0.08});
    auto image = feed.snapshot();
    cout << party_select_top.detect(image) << endl;
#endif

#if 0
    auto image = feed.snapshot();
    SomethingInBoxSlotDetector detector(COLOR_RED, true);
    detector.make_overlays(overlays);
    cout << detector.detect(image) << endl;
#endif

#if 0
    AsyncCommandSession session(scope, logger, env.realtime_dispatcher(), context.botbase());
    session.dispatch([](ProControllerContext& context){
//        pbf_controller_state(context, 0, DPAD_NONE, 128, 0, 128, 128, 255);
        pbf_press_button(context, BUTTON_A, 255, 0);
    });
    context.wait_for(std::chrono::seconds(2));
    session.dispatch([](ProControllerContext& context){
//        pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 255);
        pbf_press_button(context, BUTTON_B, 255, 0);
    });

#else
#if 0
    pbf_controller_state(context, 0, DPAD_NONE, 128, 0, 128, 128, 500);
    context.wait_for_all_requests();
    pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 500);
#endif
#endif

//    pbf_press_button(context, BUTTON_B, 500, 10);
//    pbf_controller_state(context, 0, DPAD_NONE, 128, 0, 128, 128, 500);
//    context.wait_for_all_requests();
//    pbf_wait(context, 1);
//    pbf_controller_state(context, BUTTON_B, DPAD_NONE, 128, 0, 128, 128, 500);



#if 0
    KeyboardEntryPosition point0{1, 0};
    KeyboardEntryPosition point1{1, 10};
//    uint16_t scroll_delay = 3;
//    uint16_t A_delay = 3;

    auto path0 = get_codeboard_digit_path(point0, point1);
    auto path1 = get_codeboard_digit_path(point1, point0);

    while (true){
        move_codeboard(context, path0, true);
        move_codeboard(context, path1, true);
    }
#endif
#if 0
    while (true){
        ssf_issue_scroll(context, DPAD_LEFT, 10, 6);
        ssf_issue_scroll(context, DPAD_LEFT, 4, 6);
        ssf_issue_scroll(context, DPAD_LEFT, 4, 6);
        ssf_issue_scroll(context, DPAD_LEFT, 10, 6);
        ssf_issue_scroll(context, DPAD_RIGHT, 10, 6);
        ssf_issue_scroll(context, DPAD_RIGHT, 4, 6);
        ssf_issue_scroll(context, DPAD_RIGHT, 4, 6);
        ssf_issue_scroll(context, DPAD_RIGHT, 10, 6);
    }
#endif

#if 0
    ImageRGB32 image("20221206-225502975702-NoState.png");

//    OverlayBoxScope ore_box(console, {0.930, 0.050, 0.065, 0.010});
//    extract_box_reference(image, ore_box).save("test.png");

    AdvanceDialogDetector detector;
    cout << detector.detect(image) << endl;
#endif

#if 0
    while (true){
        pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
        set_time_to_12am_from_home(console, context);
        reset_game_from_home(env, console, context, 5 * TICKS_PER_SECOND);
    }
#endif


#if 0
    ImageRGB32 image("screenshot-20221204-232949766645.png");

    MainMenuDetector detector;
    auto detection = detector.detect_location(image);
    cout << (int)detection.first << " : " << detection.second << endl;

//    GradientArrowDetector party_select_top(COLOR_GREEN, GradientArrowType::RIGHT, {0.30, 0.27, 0.10, 0.08});
//    bool detected = party_select_top.detect(image);
//    cout << detected << endl;
#endif





#if 0
//    GradientArrowDetector party_select_top(COLOR_GREEN, GradientArrowType::RIGHT, {0.30, 0.27, 0.10, 0.08});
    MainMenuDetector detector;
    while (true){
        VideoSnapshot snapshot = feed.snapshot();
        auto detection = detector.detect_location(snapshot);
//        bool detected = party_select_top.detect(snapshot);
        cout << (int)detection.first << " : " << detection.second << endl;
//        if (!detected){
//            snapshot.frame->save("test.png");
//        }
        scope.wait_for(std::chrono::milliseconds(100));
    }
#endif

#if 0
    send_program_notification(
        env.logger(), NOTIFICATION_TEST,
        COLOR_GREEN, env.program_info(),
        "Test Title",
        {
            {
                "",
                "[TestTest](https://discordtips.com/how-to-hyperlink-in-discord/)"
            },
        }
    );
#endif


//    pbf_move_left_joystick(context, 129, 128, 10000, 0);
//    pbf_move_left_joystick(context, 128, 0, 90, 0);


//    ImageRGB32 image("screenshot-20221128-084818449677.png");
//    PromptDialogDetector detector(COLOR_RED);
//    cout << detector.detect(image) << endl;


//    ssf_press_button(context, BUTTON_A, 8, 20);
//    pbf_press_button(context, BUTTON_B, 20, 105);




#if 0
    ImageRGB32 image("20221206-035546876682.jpg");
//    auto image = feed.snapshot();

    TeraLobbyReader reader;
    reader.make_overlays(overlays);
#if 0
    reader.check_ban_list(
        {
            {Language::English, "Halazea"},
        },
        image,
        true
    );
#endif
    reader.raid_code(logger, env.program_info(), image);
//    cout << (int)reader.total_players(image) << endl;
#endif



//    connect_to_internet_from_overworld(console, context);
//    day_skip_from_overworld(console, context);



//    save_game_from_overworld(console, context);


#if 0
    OverworldDetector overworld;
    overworld.make_overlays(overlays);

    auto image = feed.snapshot();
//    overworld.detect(image);
    cout << overworld.detect_ball(image) << endl;
#endif

//    wait_until();

//    ImageRGB32 image("ball-1-new.png");

//    filter_rgb32_range(image, 0xffc0c000, 0xffffff3f, Color(0), false).save("ball-template.png");


//    ImageFloatBox ball(0.890, 0.790, 0.030, 0.070);
//    ImageFloatBox radar(0.815, 0.680, 0.180, 0.310);



//    save_game_from_menu(console, context);

//    enter_alphanumeric_code(logger, context, "2VL4EP");

//    run_path(context, get_path({0, 0}, {2, 6}));
//    run_path(context, get_path({2, 6}, {0, 9}));
//    run_path(context, get_path({0, 9}, {3, 8}));



#if 0
    auto image = feed.snapshot();

    DateReader reader;
    reader.make_overlays(overlays);
    cout << reader.detect(image) << endl;
    cout << (int)reader.read_hours(logger, image) << endl;
    reader.set_hours(console, context, 1);
#endif


//    auto image = feed.snapshot();
//    DateReader reader;
//    reader.detect(image);



//    TradeStats stats;
//    trade_current_box(env, scope, NOTIFICATION_TEST, stats);


//    BoxDetector detector;
//    detector.move_cursor(console, context, BoxCursorLocation::SLOTS, 3, 3);

//    MainMenuDetector detector;
//    detector.move_cursor(console, context, MenuSide::RIGHT, 4);

#if 0
    auto image = feed.snapshot();
    BoxDetector detector;
    auto ret = detector.detect_location(image);
    cout << (int)ret.first << " | " << (int)ret.second.row << ", " << (int)ret.second.col << endl;
#endif

#if 0
    MultiConsoleErrorState state;
    TradeStats stats;
    env.run_in_parallel(
        scope,
        [&](ConsoleHandle& console, ProControllerContext& context){
            trade_current_pokemon(console, context, state, stats);
        }
    );
#endif


#if 0
    auto image = feed.snapshot();
    TeraLobbyReader detector(COLOR_RED);
    detector.make_overlays(overlays);
    cout << detector.detect(image) << endl;
    cout << detector.total_players(image) << endl;
#endif


#if 0
//    ImageRGB32 image("NoCardDetection.png");
    auto image = feed.snapshot();
    TeraCardReader detector(COLOR_RED);
    detector.make_overlays(overlays);
    cout << detector.detect(image) << endl;
    cout << detector.stars(image) << endl;
#endif


#if 0
    ImageRGB32 image("ArrowFail.png");
//    auto image = feed.snapshot();

    AdvanceDialogDetector detector;
    cout << detector.detect(image) << endl;
//    image.frame->save("test.png");
#endif


#if 0
    BattleBallReader reader(console, LANGUAGE);

    pbf_press_button(context, BUTTON_A, 20, 105);
    context.wait_for_all_requests();

    int quantity = move_to_ball(reader, console, context, "poke-ball");
    cout << "quantity = " << quantity << endl;
#endif


#if 0
    BattleBallReader reader(console, Language::English);
    auto image = feed.snapshot();
    cout << reader.read_quantity(image) << endl;
    cout << reader.read_ball(image) << endl;
#endif

#if 0
//    ImageRGB32 image("screenshot-20221120-001408323077.png");
    ImageRGB32 image("screenshot-20221118-160757832016.png");
    PokemonSummaryDetector detector;
    cout << detector.detect(image) << endl;
#endif

//    auto image = feed.snapshot();
//    PokemonSummaryDetector detector;
//    cout << detector.detect(image) << endl;


#if 0
    auto image = feed.snapshot();
    ImageStats stats = image_stats(extract_box_reference(image, ImageFloatBox{0.30, 0.33, 0.40, 0.02}));
    cout << stats.average << stats.stddev << endl;
#endif

#if 0
    ImageRGB32 image("cursor_basic_00q.png");
    GradientArrowDetector detector({0, 0, 1, 1});

    auto hits = detector.detect_all(image);
    cout << "hits = " << hits.size() << endl;
    for (auto& item : hits){
        extract_box_reference(image, item).save("test.png");
    }
#endif


#if 0
    ImageRGB32 image("screenshot-20221121-070043212515.png");

    WhiteButtonDetector next_button(WhiteButton::ButtonA, {0.8, 0.9, 0.2, 0.1}, COLOR_RED);
    cout << next_button.detect(image) << endl;
#endif

#if 0
    ImageRGB32 image("screenshot-20221118-211428612196.png");

    TeraCardReader detector;
    cout << detector.detect(image) << endl;
#endif

#if 0
    ImageRGB32 image("screenshot-20221122-022035906115.png");

    MoveSelectDetector detector;
    cout << detector.detect(image) << endl;
#endif


#if 0
    AddToPartyDetector detector;
    cout << detector.detect(feed.snapshot()) << endl;
#endif



//    home_to_date_time(context, false, false);

//    save_game_from_overworld(console, context);


#if 0
    auto image = feed.snapshot();

    PokemonSummaryDetector detector;
    cout << detector.detect(image) << endl;
    cout << detector.is_shiny(image) << endl;
#endif

#if 0
    RaidShinyStarDetector detector(overlay);
    wait_until(
        console, context,
        WallClock::max(),
        {detector}
    );
#endif



#if 0
    ImageRGB32 image("ShinyRaid.png");

    ImageViewRGB32 cropped = extract_box_reference(image, ImageFloatBox{0.5, 0.1, 0.4, 0.7});
    cropped.save("test-cropped.png");


    ImageRGB32 filtered = filter_rgb32_range(cropped, 0xff804040, 0xffffffff, Color(0xffffff00), true);
    filtered.save("test-filtered.png");
#endif

#if 0
    uint8_t year = MAX_YEAR;
    while (true){
        pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY);
        home_roll_date_enter_game_autorollback(console, context, year);
        pbf_mash_button(context, BUTTON_A, 250);
    }
#endif

#if 0
    ImageRGB32 image("GradientArrowHorizontal-Template.png");

    ImageRGB32 rotated(image.height(), image.width());

    for (size_t r = 0; r < image.height(); r++){
        for (size_t c = 0; c < image.width(); c++){
            rotated.pixel(r, c) = image.pixel(c, r);
        }
    }
    rotated.save("GradientArrowVertical-Template.png");
#endif


#if 0
    auto image = feed.snapshot();
    ImageViewRGB32 cropped = extract_box_reference(image, ImageFloatBox{0.500, 0.555, 0.310, 0.070});
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_euclidean(cropped, 0xff757f9c, 100);
//    cout << matrix.dump() << endl;

    matrix.invert();
    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(100);
    WaterfillObject object;
    while (iter->find_next(object, false)){
//        extract_box_reference(image, object).save("Arrow-Template.png");
        cout << object.area << endl;
    }
#endif


#if 0
    auto image = feed.snapshot();

    ImageViewRGB32 cropped = extract_box_reference(image, ImageFloatBox{0.500, 0.555, 0.310, 0.070});
//    ImageRGB32 filtered = filter_rgb32_range(cropped, 0xffc00000, 0xffffffff, Color(0x00000000), true);
    size_t pixels;
    ImageRGB32 filtered = filter_rgb32_euclidean(pixels, cropped, 0xff757f9c, 100, Color(0x00000000), true);
    cout << "pixels = " << pixels << endl;
    filtered.save("test.png");
#endif


#if 0
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff808000, 0xffffff80);

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(100);
    WaterfillObject object;
    while (iter->find_next(object, false)){
        extract_box_reference(image, object).save("Arrow-Template.png");
    }
#endif

#if 0
    ImageRGB32 image("WhiteButtonA.png");
    ImageRGB32 filtered = filter_rgb32_range(image, 0xff000000, 0xff7f7f7f, Color(0x00000000), true);

    for (size_t r = 0; r < image.height(); r++){
        for (size_t c = 0; c < image.width(); c++){
            if (8 < c && c < 30 && 8 < r && r < 30){
                filtered.pixel(c, r) = image.pixel(c, r);
            }
        }
    }
    filtered.save("WhiteButtonA-processed.png");
#endif

#if 0
    WhiteButtonFinder next_button(WhiteButton::ButtonA, console.overlay(), {0.9, 0.9, 0.1, 0.1});
    wait_until(
        console, context,
        std::chrono::seconds(60),
        {next_button}
    );
#endif


#if 0
    TeraCardReader reader;
//    auto image = ImageRGB32("ErrorDumps/20221115-234552197119-ReadStarsFailed.png");
    auto image = feed.snapshot();
    cout << reader.detect(image) << endl;
    cout << "stars = " << reader.stars(image) << endl;

//    BattleMenuDetector battle_menu;
//    cout << battle_menu.detect(image) << endl;
#endif

//    OverlayBoxScope box(overlay, COLOR_RED, {0.4, 0.4, 0.2, 0.2}, "asdf qwer sdfg");


//    reset_game_to_gamemenu(console, context);


#if 0
    ImageRGB32 image("Arrow.png");

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff808080, 0xffffffff);

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(20);
    WaterfillObject object;
    while (iter->find_next(object, false)){
        extract_box_reference(image, object).save("Arrow-Template.png");
    }
#endif


//    YCommMenuDetector detector(true);
//    HomeMenuDetector detector;
//    cout << detector.detect(image) << endl;
//    cout << detector.detect(feed.snapshot()) << endl;



#if 0
    while (true){
        pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
        reset_game_from_home(env, console, context);
    }
#endif



#if 0
    overlay.add_log("asdfasdf", COLOR_RED);

    for (int c = 0; c < 20; c++){
        overlay.add_log("qwerqwer", COLOR_GREEN);
        scope.wait_for(std::chrono::milliseconds(1000));
    }

    scope.wait_for(std::chrono::milliseconds(5000));
    cout << "clear" << endl;
    overlay.clear_log();
#endif

#if 0
    OverlayTextScope text(overlay, "hello world", 0.5, 0.5, 10, COLOR_WHITE);

    overlay.add_log_text("asdfasdf", COLOR_RED);

    OverlayStat stat0;
    OverlayStat stat1;
    OverlayStat stat2;
    overlay.add_stat(stat0);
    overlay.add_stat(stat1);
    overlay.add_stat(stat2);

    for (size_t c = 0;; c++){
        stat2.set_text(std::to_string(c));
        scope.wait_for(std::chrono::milliseconds(100));
    }
#endif


#if 0
    ImageFloatBox box(0.21, 0.80, 0.53, 0.17);

    VideoSnapshot frame = feed.snapshot();
    ImageViewRGB32 image = extract_box_reference(frame, box);

    ImageRGB32 filtered = to_blackwhite_rgb32_range(image, 0xff007f7f, 0xffc0ffff, false);
    filtered.save("test.png");

    PokeballNameReader::instance().read_substring(
        logger, Language::English,
        image,
        {{0xff007f7f, 0xff80ffff}}, 0
    );
#endif


#if 0
    ImageRGB32 image("screenshot-20221107-210754107968.png");
//    auto image = feed.snapshot();
    HomeMenuDetector detector;
//    UpdatePopupDetector detector;
    VideoOverlaySet overlays(overlay);
    detector.make_overlays(overlays);
    cout << detector.detect(image) << endl;
#endif

//    ImageRGB32 image("ExclamationFalsePositive.png");
//    find_exclamation_marks(image);

//    HomeMenuDetector detector;
//    cout << detector.detect(image) << endl;


#if 0
    InferenceBoxScope left_mon_white(console, {0.708, 0.070, 0.005, 0.028});
    InferenceBoxScope left_mon_hp(console, {0.500, 0.120, 0.18, 0.005});
    InferenceBoxScope left_name(console, {0.467, 0.06, 0.16, 0.050});
    InferenceBoxScope right_name(console, {0.740, 0.06, 0.16, 0.050});
#endif

//    ImageRGB32 image("Paralyzed.png");

#if 0
    BattleMenuDetector detector(BattleType::STANDARD);
    VideoOverlaySet overlays(overlay);
    detector.make_overlays(overlays);
    cout << detector.detect(image) << endl;
#endif

#if 0
    BattleMenuWatcher watcher(BattleType::STANDARD);
    BotBaseContext context(scope, console.botbase());
    wait_until(
        console, context, std::chrono::seconds(60),
        {
            {watcher}
        }
    );
#endif


#if 0
//    ImageRGB32 image("SV-BattleMenu.png");
//    ImageRGB32 image("SV-Hair.png");
//    image = image.scale_to(1920, 1080);


//    extract_box_reference(image, ImageFloatBox({0.7, 0.6, 0.2, 0.1})).save("tmp.png");

//    ImageFloatBox box(0.5, 0.5, 0.4, 0.5);
    ImageFloatBox box(0.0, 0.0, 1.0, 1.0);


    VideoOverlaySet set(overlay);
    WhiteButtonFinder white_button_detector0(WhiteButton::ButtonA, overlay, box);
    WhiteButtonFinder white_button_detector1(WhiteButton::ButtonB, overlay, box);
    WhiteButtonFinder white_button_detector2(WhiteButton::ButtonY, overlay, box);
    WhiteButtonFinder white_button_detector3(WhiteButton::ButtonMinus, overlay, box);
    DialogArrowFinder dialog_arrow_detector(overlay, box);
    GradientArrowFinder gradient_arrow_detector(overlay, box);
    dialog_arrow_detector.make_overlays(set);
    gradient_arrow_detector.make_overlays(set);
    BattleMenuFinder battle_menu;
    battle_menu.make_overlays(set);

    while (true){
        scope.wait_for(std::chrono::milliseconds(50));
        VideoSnapshot snapshot = feed.snapshot();

        white_button_detector0.process_frame(snapshot, current_time());
        white_button_detector1.process_frame(snapshot, current_time());
        white_button_detector2.process_frame(snapshot, current_time());
        white_button_detector3.process_frame(snapshot, current_time());
        dialog_arrow_detector.process_frame(snapshot, current_time());
        gradient_arrow_detector.process_frame(snapshot, current_time());
        battle_menu.process_frame(snapshot, current_time());
    }
#endif

#if 0
    BotBaseContext context(scope, console.botbase());
    wait_until(
        console, context, std::chrono::seconds(60),
        {
            {detector}
        },
        std::chrono::seconds(50)
    );
#endif






    scope.wait_for(std::chrono::seconds(60));


}




#if 0
struct RaidShinyStar{
    double alpha;
    WaterfillObject object;
};


class RaidShinyStarDetector : public VisualInferenceCallback{
    static constexpr double ALPHA_THRESHOLD = 1.0;

public:
    RaidShinyStarDetector(VideoOverlay& overlay)
        : VisualInferenceCallback("RaidShinyStarDetector")
        , m_overlay(overlay)
        , m_box(overlay, {0.5, 0.1, 0.4, 0.7})
    {}

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    std::vector<RaidShinyStar> process_frame(const ImageViewRGB32& frame);

private:
    double test_object(const ImageViewRGB32& image, const WaterfillObject& object);


private:
    VideoOverlay& m_overlay;
    OverlayBoxScope m_box;
    std::deque<OverlayBoxScope> m_stars;
};


void RaidShinyStarDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool RaidShinyStarDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    process_frame(frame);
    return false;
}

double RaidShinyStarDetector::test_object(const ImageViewRGB32& image, const WaterfillObject& object){
    double aspect_ratio = object.aspect_ratio();
    if (aspect_ratio < 0.9 || aspect_ratio > 1.1){
        return 0;
    }

    double area_ratio = object.area_ratio();
    if (area_ratio < 0.5 || area_ratio > 0.8){
        return 0;
    }

    //  Check that center of gravity is centered.
    double center_of_gravity_x = object.center_of_gravity_x();
    double center_of_gravity_y = object.center_of_gravity_y();
    double center_x = object.min_x + object.width() * 0.5;
    double center_y = object.min_y + object.height() * 0.5;

    double center_shift_x = center_x - center_of_gravity_x;
    double center_shift_y = center_y - center_of_gravity_y;
    center_shift_x *= center_shift_x;
    center_shift_y *= center_shift_y;

    double max_x_sqr = object.width() * 0.1;
    double max_y_sqr = object.height() * 0.1;
    max_x_sqr *= max_x_sqr;
    max_y_sqr *= max_y_sqr;

    if (center_shift_x > max_x_sqr){
        return 0;
    }
    if (center_shift_y > max_y_sqr){
        return 0;
    }

    return 1.0;
}


std::vector<RaidShinyStar> RaidShinyStarDetector::process_frame(const ImageViewRGB32& frame){

    ImageViewRGB32 cropped = extract_box_reference(frame, m_box);

    std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(cropped, {
        {0xff808080, 0xffffffff},
        {0xff909090, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
        {0xffb0b0b0, 0xffffffff},
        {0xffc0c0c0, 0xffffffff},
        {0xffd0d0d0, 0xffffffff},
        {0xffe0e0e0, 0xffffffff},

        {0xff804040, 0xffffffff},
        {0xff905050, 0xffffffff},
        {0xffa06060, 0xffffffff},

        {0xff408040, 0xffffffff},
        {0xff509050, 0xffffffff},
        {0xff60a060, 0xffffffff},

        {0xff404080, 0xffffffff},
        {0xff505090, 0xffffffff},
        {0xff6060a0, 0xffffffff},
    });

//    std::vector<RaidShinyStar>
    std::vector<RaidShinyStar> stars;

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
    WaterfillObject object;
    for (PackedBinaryMatrix& matrix : matrices){
        session->set_source(matrix);
        auto iter = session->make_iterator(10);
        while (iter->find_next(object, false)){
            double alpha = test_object(cropped, object);
            if (alpha >= ALPHA_THRESHOLD){
                stars.emplace_back(RaidShinyStar{alpha, object});
            }
        }
    }






    //  Redraw the boxes.
    m_stars.clear();
    for (const RaidShinyStar& star : stars){
        m_stars.emplace_back(m_overlay, translate_to_parent(frame, m_box, star.object), COLOR_BLUE);
    }
    return stars;
}

#endif




}
}




