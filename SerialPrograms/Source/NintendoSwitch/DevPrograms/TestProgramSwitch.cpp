/*  Test Program (Switch)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "TestProgramSwitch.h"

//#include <immintrin.h>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "Common/Cpp/Concurrency/PeriodicScheduler.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/InferenceInfra/InferenceSession.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "CommonFramework/InferenceInfra/VisualInferencePivot.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_IVCheckerReader.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleBallReader.h"
#include "PokemonLA/Programs/PokemonLA_LeapPokemonActions.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxGenderDetector.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapZoomLevelReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "CommonFramework/Inference/FrozenImageDetector.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ShinySymbolDetector.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DetectHome.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/PokemonSV_DialogArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_ScalarButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DateReader.h"
#include "NintendoSwitch/Programs/NintendoSwitch_FastCodeEntry.h"
#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_BasicCatcher.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSV/Inference/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/PokemonSV_PokemonSummaryReader.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSV/Inference/PokemonSV_PostCatchDetector.h"
#include "PokemonSV/Inference/PokemonSV_BattleBallReader.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/PokemonSV_BoxDetection.h"
#include "PokemonSV/Programs/Trading/PokemonSV_TradeRoutines.h"


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


StringSelectDatabase make_database(){
    StringSelectDatabase ret;
    for (size_t c = 0; c < 1000; c++){
        ret.add_entry(StringSelectEntry("slug" + std::to_string(c), "Display " + std::to_string(c)));
    }
    return ret;
}
const StringSelectDatabase& test_database(){
    static StringSelectDatabase database = make_database();
    return database;
}


TestProgram_Descriptor::TestProgram_Descriptor()
    : MultiSwitchProgramDescriptor(
        "NintendoSwitch:TestProgram",
        "Nintendo Switch", "Test Program (Switch)",
        "",
        "Test Program (Switch)",
        FeedbackType::OPTIONAL_, true,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}


TestProgram::TestProgram()
    : LANGUAGE(
        "<b>OCR Language:</b>",
        { Language::English },
        LockWhileRunning::LOCKED,
        false
    )
    , STATIC_TEXT("Test text...")
    , SELECT("String Select", test_database(), LockWhileRunning::LOCKED, 0)
    , NOTIFICATION_TEST("Test", true, true, ImageAttachmentMode::JPG)
    , NOTIFICATIONS({
        &NOTIFICATION_TEST,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(STATIC_TEXT);
    PA_ADD_OPTION(SELECT);
    PA_ADD_OPTION(NOTIFICATIONS);
}


//using namespace Kernels;
using namespace Kernels::Waterfill;

//using namespace PokemonLA;










void TestProgram::program(MultiSwitchProgramEnvironment& env, CancellableScope& scope){
    using namespace Kernels;
    using namespace Kernels::Waterfill;
    using namespace OCR;
    using namespace NintendoSwitch;
    using namespace Pokemon;
//    using namespace PokemonSwSh;
//    using namespace PokemonBDSP;
//    using namespace PokemonLA;
    using namespace PokemonSV;

    [[maybe_unused]] Logger& logger = env.logger();
    [[maybe_unused]] ConsoleHandle& console = env.consoles[0];
//    [[maybe_unused]] BotBase& botbase = env.consoles[0];
    [[maybe_unused]] VideoFeed& feed = env.consoles[0];
    [[maybe_unused]] VideoOverlay& overlay = env.consoles[0];
    BotBaseContext context(scope, console.botbase());
    VideoOverlaySet overlays(overlay);


    pbf_move_left_joystick(context, 128, 255, 80, 0);
    pbf_move_left_joystick(context, 128, 0, 90, 0);


//    ImageRGB32 image("screenshot-20221128-084818449677.png");
//    PromptDialogDetector detector(COLOR_RED);
//    cout << detector.detect(image) << endl;


//    ssf_press_button(context, BUTTON_A, 8, 20);
//    pbf_press_button(context, BUTTON_B, 20, 105);



#if 0
    ImageRGB32 image("BadCode-0.jpg");

    TeraLobbyReader reader;
    reader.raid_code(logger, env.program_info(), image);
#endif



//    connect_to_internet_from_overworld(console, context);
//    day_skip_from_overworld(console, context);



//    save_game_from_overworld(console, context);


#if 0
    OverworldDetector overworld;
    overworld.make_overlays(overlays);

    auto image = feed.snapshot();
//    overworld.detect(image);
    overworld.detect_ball(image);
#endif

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
        [&](ConsoleHandle& console, BotBaseContext& context){
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
//    HomeDetector detector;
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
    HomeDetector detector;
//    UpdateMenuDetector detector;
    VideoOverlaySet overlays(overlay);
    detector.make_overlays(overlays);
    cout << detector.detect(image) << endl;
#endif

//    ImageRGB32 image("ExclamationFalsePositive.png");
//    find_exclamation_marks(image);

//    HomeDetector detector;
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




