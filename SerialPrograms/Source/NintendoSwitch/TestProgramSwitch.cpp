/*  Test Program (Switch)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "TestProgramSwitch.h"

//#include <immintrin.h>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/AsyncDispatcher.h"
#include "Common/Cpp/PeriodicScheduler.h"
#include "Common/Cpp/PrettyPrint.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/InferenceInfra/InferenceSession.h"
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

#include <QVideoFrame>

//#include <Windows.h>
#include <iostream>
using std::cout;
using std::endl;


//#include "../Internal/SerialPrograms/NintendoSwitch_Commands_ScalarButtons.h"
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
        { Language::English }, false
    )
    , STATIC_TEXT("Test text...")
    , SELECT("String Select", test_database(), 0)
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
    using namespace Pokemon;
//    using namespace PokemonSwSh;
//    using namespace PokemonBDSP;
    using namespace PokemonLA;

    [[maybe_unused]] Logger& logger = env.logger();
    [[maybe_unused]] ConsoleHandle& console = env.consoles[0];
//    [[maybe_unused]] BotBase& botbase = env.consoles[0];
    [[maybe_unused]] VideoFeed& feed = env.consoles[0];
    [[maybe_unused]] VideoOverlay& overlay = env.consoles[0];



    ImageRGB32 image("screenshot-20220814-170240979105.png");

    PokemonSwSh::SummaryShinySymbolDetector detector(logger, overlay);

    cout << detector.detect(image) << endl;
//    detector.wait_for_detection(scope, feed);




#if 0
    for (size_t c = 0; c < 1000; c++){
        STATIC_TEXT.set_text("Timer: " + std::to_string(c));
        scope.wait_for(std::chrono::milliseconds(100));
    }
#endif


//    ImageRGB32 image("screenshot-20220807-171750797516.png");


//    BattleMenuDetector detector(logger, overlay, true);
//    cout << detector.process_frame(image, current_time()) << endl;


//    ImageRGB32 image("04_True_True_eng_hippowdon_NotShiny_NotAlpha_Female.png");

//    GreyDialogDetector detector;
//    cout << detector.detect(image) << endl;


//    std::shared_ptr<const ImageRGB32> image = feed.snapshot();

//    find_shiny_symbols(image);



#if 0
    ImageRGB32 image("screenshot-20220725-170822724101.png");

    NintendoSwitch::PokemonLA::FlagTracker tracker(logger, overlay);

    tracker.process_frame(image, current_time());
#endif


#if 0
    BotBaseContext context(scope, console.botbase());
    wait_until(
        console, context, std::chrono::seconds(60),
        {
            {tracker}
        },
        std::chrono::seconds(50)
    );
#endif


#if 0
    std::shared_ptr<const ImageRGB32> image = feed.snapshot();

    cout << image->width() << " x " << image->height() << endl;

    cout << image->save("test.png") << endl;
#endif

#if 0
    VideoOverlaySet overlays(overlay);

    MaxLairInternal::BattleMenuDetector detector;
    detector.make_overlays(overlays);
    bool ret = detector.detect(ImageRGB32("screenshot-20220722-231109920242.png"));
    cout << ret << endl;
#endif


#if 0
    StandardBattleMenuDetector detector(true);

    bool ret = detector.detect(ImageRGB32("screenshot-20220722-231109920242.png"));
    cout << ret << endl;
#endif


#if 0
    FrozenImageDetector detector(std::chrono::seconds(10), 20);
    BotBaseContext context(scope, console.botbase());
    int ret = wait_until(
        console, context, std::chrono::seconds(600),
        { detector }
    );
    cout << "ret = " << ret << endl;
#endif



#if 0
    InferenceBoxScope box0(env.consoles[0], {0.925, 0.100, 0.015, 0.030});

    QImage image("screenshot-20220717-204925545835.png");
    ImageViewRGB32 cropped = extract_box_reference(image, box0);
    ImageStats stats = image_stats(cropped);
    cout << stats.average << stats.stddev << endl;
    cropped.save("test.png");
#endif



#if 0
    MaxLairInternal::PokemonSwapMenuReader reader(logger, overlay, Language::English);

    int8_t pp[4];
    reader.read_pp(QImage("screenshot-20220717-125742133117.png"), pp);
#endif


//    read_map_zoom_level(feed.snapshot());




#if 0
    QImage image("MiraBox-Qt5.png");

    BattleMenuDetector detector(BattleType::STARTER);
    cout << detector.detect(image) << endl;
#endif


//    GreyDialogDetector detector;
//    cout << detector.detect(image) << endl;



#if 0
    QImage image("screenshot-20220703-124134719005.png");


    InferenceBoxScope box(overlay, 0.83, 0.95, 0.11, 0.027);

    cout << is_pokemon_selection(overlay, image) << endl;
#endif

#if 0
    {
        QImage image("screenshot-20220620-182701565112.png");
        EggHatchGenderFilter gender = read_gender_from_box(console, console, image);
        cout << (int)gender << endl;
    }
    {
        QImage image("screenshot-20220620-182704790542.png");
        EggHatchGenderFilter gender = read_gender_from_box(console, console, image);
        cout << (int)gender << endl;
    }
#endif

#if 0
    QImage image("screenshot-20220613-170430686597.png");

    VideoOverlaySet set(overlay);
    PokemonSwSh::MaxLairInternal::BattleMenuDetector detector;
    detector.make_overlays(set);

    cout << detector.detect(image) << endl;
#endif


#if 0
    FlagTracker tracker(logger, overlay);

    QImage image("test-57.png");
    tracker.process_frame(image, current_time());
#endif


//    InferenceBoxScope box(overlay, 0.843, 0.96, 0.075, 0.005);

//    is_pokemon_selection(overlay, feed.snapshot().frame);



#if 0
    BotBaseContext context(scope, console.botbase());


    PokemonDetails pokemon = get_pokemon_details(console, context, LANGUAGE);
    cout << set_to_str(pokemon.name_candidates) << endl;
    cout << get_gender_str(pokemon.gender) << endl;
    cout << "is_alpha = " << pokemon.is_alpha << endl;
    cout << "is_shiny = " << pokemon.is_shiny << endl;
#endif



#if 0
    PokemonBDSP::BattleBallReader reader(console, Language::English);
    reader.read_ball(feed.snapshot());
    reader.read_quantity(feed.snapshot());
#endif



//    PokemonBDSP::IVCheckerReaderScope reader(overlay, Language::English);
//    reader.read(logger, feed.snapshot());



#if 0
    VisualInferencePivot pivot(scope, feed, env.inference_dispatcher());

    MountTracker tracker(logger, MountDetectorLogging::LOG_ONLY);

    pivot.add_callback(scope, tracker, std::chrono::milliseconds(500));
#endif


//    scope.wait_for(std::chrono::seconds(60));

#if 0
    std::function<void()> callback0 = []{ cout << "asdf" << endl; };
    std::function<void()> callback1 = []{ cout << "qwer" << endl; };

    PeriodicScheduler scheduler;
    scheduler.add_event(&callback0, std::chrono::seconds(2));
    scheduler.add_event(&callback1, std::chrono::seconds(3));
    while (true){
        void* ptr = scheduler.next_event();
        if (ptr != nullptr){
            (*(std::function<void()>*)ptr)();
        }
    }
#endif



#if 0
    {
        std::unique_ptr<CancellableScope> scope0(new CancellableScope());
//        std::unique_ptr<CancellableScope> scope1;
//        std::unique_ptr<CancellableScope> scope2;
        auto task = env.inference_dispatcher().dispatch([&]{
            CancellableScope scope1(*scope0);
            CancellableScope scope2(scope1);
//            scope1 = std::make_unique<CancellableScope>(*scope0);
//            scope2 = std::make_unique<CancellableScope>(*scope1);
            scope2.wait_for(std::chrono::seconds(10));
            scope1.wait_for(std::chrono::seconds(4));


        });
        cout << "waiting..." << endl;

        scope0->wait_for(std::chrono::seconds(4));
        cout << "stopping" << endl;
//        scope0.cancel();
        scope0.reset();

//        scope0.wait_for(std::chrono::seconds(20));
    }
    cout << "stopped" << endl;
#endif



#if 0
//    QImage image("screenshot-20220327-190703102304.png");
    QImage image = feed.snapshot();
    ArcPhoneDetector detector(console, console, std::chrono::milliseconds(0), true);
    detector.process_frame(image, current_time());
#endif



#if 0
    InferenceBoxScope box(overlay, 0.010, 0.700, 0.050, 0.100);
    QImage image = extract_box_copy(feed.snapshot(), box);


    auto matrix = compress_rgb32_to_binary_range(image, 0xff808080, 0xffffffff);

//    auto session = make_WaterfillSession(matrix);
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20);
    cout << "objects = " << objects.size() << endl;
    int c = 0;
    for (const WaterfillObject& object : objects){
        extract_box_reference(image, object).save("test-" + QString::number(c++) + ".png");
    }
#endif



#if 0
    QImage image("ArcPhoneTriggered-2.png");

    ArcPhoneDetector detector(console, console, std::chrono::milliseconds(0), true);
    cout << detector.process_frame(image, current_time()) << endl;
#endif



//    change_mount(console, MountState::WYRDEER_ON);

#if 0
    PokemonBDSP::ShinySparkleSetBDSP set;
    ShinySparkleTracker tracker(logger, overlay, set, {0, 0, 1, 1});

    AsyncVisualInferenceSession visual(env, console, console, console);
    visual += tracker;
//    tracker.process_frame(feed.snapshot(), current_time());
#endif


#if 0
    FlagTracker tracker(logger, overlay);


//    QImage src("20220315-054734853907.jpg");
    QImage src("20220315-055335301551.jpg");
    auto start = current_time();
    uint64_t c = 0;
    while (current_time() - start < std::chrono::seconds(10)){
        tracker.process_frame(src, current_time());
        env.check_stopping();
        c++;
//        break;
    }
    auto elapsed = current_time() - start;
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
    cout << "iterations/sec = " << (double)c / micros.count() * 1000000 << endl;
#endif


#if 0
    QImage image("ButtonMinus-Original-test.png");
    image = image.convertToFormat(QImage::Format_ARGB32);

    int width = image.width();
    int height = image.height();
    int plus_min_x = 3;
    int plus_max_x = 24;
    int plus_min_y = 9;
    int plus_max_y = 18;
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
            if (plus_min_x < c && c < plus_max_x && plus_min_y < r && r < plus_max_y){
                continue;
            }
            QRgb pixel = image.pixel(c, r);
            if (qRed(pixel) < 128 || qGreen(pixel) < 128 || qBlue(pixel) < 128){
                image.setPixel(c, r, 0);
            }
        }
    }

    image.save("ButtonMinus-Template.png");
#endif


#if 0
    QImage image("ArcPhoneTriggered-31.png");
//    QImage image("screenshot-20220308-225539293411.png");
    ArcPhoneDetector detector(console, console, std::chrono::milliseconds(0), true);
    detector.process_frame(image, current_time());
#endif


#if 0
    change_mount(console, MountState::BRAVIARY_ON);

    pbf_move_left_joystick(context, 160, 0, 160, 0);
    pbf_mash_button(context, BUTTON_B, 375);
#endif



#if 0
    QImage image("screenshot-20220320-021110586101.png");
//    QRgb pixel = image.pixel(1848, 761);
//    cout << qRed(pixel) << ", " << qGreen(pixel) << ", " << qBlue(pixel) << endl;

//    QImage image = feed.snapshot();

    MountDetector detector;
//    MountState state = detector.detect(image);

//    if (state == MountState::NOTHING){
//        image.save("test.png");
//    }
#endif









#if 0
    QImage image("screenshot-20220312-205532347068.png");

    InferenceBoxScope box0(overlay, 0.50, 0.450, 0.40, 0.042);
    InferenceBoxScope box1(overlay, 0.50, 0.492, 0.40, 0.042);
    InferenceBoxScope box2(overlay, 0.50, 0.350, 0.40, 0.400);

    extract_box(image, box0).save("test-0.png");
    extract_box(image, box1).save("test-1.png");
    extract_box(image, box2).save("test-2.png");
#endif



#if 0
    for (size_t c = 0; c < 10; c++){
        QImage image("Digit-" + QString::number(c) + "-Original.png");

        image = image.convertToFormat(QImage::Format::Format_ARGB32);
        uint32_t* ptr = (uint32_t*)image.bits();
        size_t words = image.bytesPerLine() / sizeof(uint32_t);
        for (int r = 0; r < image.height(); r++){
            for (int c = 0; c < image.width(); c++){
                uint32_t& pixel = ptr[r * words + c];
                uint32_t red = qRed(pixel);
                uint32_t green = qGreen(pixel);
                uint32_t blue = qBlue(pixel);
                if (red < 0xa0 || green < 0xa0 || blue < 0xa0){
                    pixel = 0x00000000;
                }
            }
        }
        image.save("Digit-" + QString::number(c) + "-Template.png");
    }
#endif





#if 0
//    QImage image("Distance-test.png");

    QImage frame = feed.snapshot();

    FlagTracker tracker(console, console);
    tracker.process_frame(frame, current_time());

    double distance, flag_x, flag_y;
    tracker.get(distance, flag_x, flag_y);

//    cout << distance << endl;
#endif



#if 0
    ImageFloatBox box(flag_x - 0.017, flag_y - 0.055, 0.032, 0.025);
    QImage image = extract_box(frame, box);



    int c = 0;

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xff808080, 0xffffffff);
//    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, 0xffd0d0d0, 0xffffffff);
    WaterfillIterator finder(matrix, 30);
    WaterfillObject object;
    while (finder.find_next(object)){
        //  Skip anything that touches the edge.
        if (object.min_x == 0 || object.min_y == 0 ||
            object.max_x + 1 == matrix.width() || object.max_y + 1 == matrix.height()
        ){
            continue;
        }
//        extract_box(image, object).save("image-" + QString::number(c++) + ".png");
        read_digit(image, object);
    }
#endif






//    goto_camp_from_jubilife(env, console, WarpSpot::ICELANDS_ICEPEAK_ARENA);

#if 0
    QImage image("screenshot-20220309-005426729947.png");

    MountDetector detector;
    detector.detect(image);
#endif


#if 0
    pbf_move_left_joystick(context, 0, 0, 50, 0);
    pbf_press_button(context, BUTTON_B, 20, 250);
    pbf_mash_button(context, BUTTON_ZL, 250);
    pbf_press_button(context, BUTTON_HOME, 20, 230);
#endif

#if 0
    {
        QImage image("screenshot-20220306-163207833403.png");
//        QImage image("screenshot-20220306-172029164014.png");

        DialogSurpriseDetector detector(logger, overlay, true);
        detector.process_frame(image, current_time());
    }
#endif
#if 0
    {
        QImage image("screenshot-20220302-094034596712.png");

        DialogDetector detector(logger, overlay, true);
        detector.process_frame(image, current_time());
    }
    {
        QImage image("screenshot-Gin");

        DialogDetector detector(logger, overlay, true);
        detector.process_frame(image, current_time());
    }
#endif


#if 0
    InferenceBoxScope box0(console, {0.925, 0.100, 0.014, 0.030});

//    QImage screen("screenshot-20220228-121927882824.png");
    QImage screen = feed.snapshot();

    QImage image = extract_box(screen, box0);
    ImageStats stats = image_stats(image);
    cout << stats.average << stats.stddev << endl;
    bool ok = is_white(stats);
    cout << ok << endl;
#endif

//    throw UserSetupError(env.logger(), "asdf");
//    throw OperationFailedException(env.logger(), "asdf");



//    FlagNavigationAir session(env, console);
//    session.run_session();



//    goto_camp_from_overworld(env, console);


//    InferenceBoxScope box(console, {0.450, 0.005, 0.040, 0.010});
//    ImageStats stats = image_stats(extract_box_reference(console.video().snapshot(), box));
//    cout << stats.average << stats.stddev << endl;



#if 0
    pbf_press_dpad(context, DPAD_UP, 20, 480);
    pbf_press_button(context, BUTTON_A, 20, 480);
    pbf_press_button(context, BUTTON_B, 20, 230);
    pbf_press_button(context, BUTTON_B, 20, 230);
#endif


#if 0
    auto& context = console;

                    pbf_move_left_joystick(context, 0, 212, 50, 0);
                    pbf_press_button(context, BUTTON_B, 500, 80);

                    pbf_move_left_joystick(context, 224, 0, 50, 0);
                    pbf_press_button(context, BUTTON_B, 350, 80);

                    pbf_move_left_joystick(context, 0, 64, 50, 0);
                    pbf_press_button(context, BUTTON_B, 250, 80);

                    pbf_move_left_joystick(context, 0, 96, 50, 0);
                    pbf_press_button(context, BUTTON_B, 500, 0);
#endif


#if 0
    VideoOverlaySet set(overlay);
    DialogDetector detector(console, console);
    detector.make_overlays(set);
    detector.process_frame(feed.snapshot(), current_time());
#endif




#if 0
    InferenceBoxScope box0(overlay, {0.010, 0.700, 0.050, 0.100});
    QImage image = extract_box(feed.snapshot(), box0);

    ArcPhoneDetector detector(console, console, std::chrono::milliseconds(200), true);
    detector.process_frame(image, current_time());
    wait_until(
        env, console, std::chrono::seconds(10),
        { &detector }
    );
#endif


//    from_professor_return_to_jubilife(env, console);



//    InferenceBoxScope box0(overlay, {0.900, 0.955, 0.080, 0.045});
//    InferenceBoxScope box1(overlay, {0.500, 0.621, 0.300, 0.043});



//    EscapeFromAttack session(env, console);
//    session.run_session();
//    cout << "Done escaping!" << endl;


#if 0
    MapDetector detector;
    VideoOverlaySet overlays(overlay);
    detector.make_overlays(overlays);
#endif

#if 0
    ButtonDetector detector(
        console, console,
        ButtonType::ButtonA,
        {0.55, 0.40, 0.20, 0.40},
        std::chrono::milliseconds(200), true
    );
    VideoOverlaySet overlays(overlay);
    detector.make_overlays(overlays);

    detector.process_frame(feed.snapshot(), current_time());
#endif


#if 0
    UnderAttackWatcher watcher;
    wait_until(
        env, console,
         std::chrono::seconds(60),
         { &watcher }
    );
#endif


//    InferenceBoxScope box(overlay, 0.49, 0.07, 0.02, 0.03);
//    ImageStats stats = image_stats(extract_box_reference(feed.snapshot(), box));
//    cout << stats.average << stats.stddev << endl;


//    return_to_jubilife_from_overworld(env, console);







#if 0
    pbf_move_right_joystick(context, 0, 128, 145, 0);
    pbf_move_left_joystick(context, 128, 0, 50, 0);
    pbf_press_button(context, BUTTON_B, 500, 125);

    pbf_move_right_joystick(context, 255, 128, 45, 0);
    pbf_move_left_joystick(context, 128, 0, 50, 0);
    pbf_press_button(context, BUTTON_B, 420, 125);

    pbf_move_right_joystick(context, 0, 128, 100, 0);
    pbf_move_left_joystick(context, 128, 0, 50, 0);
    pbf_press_button(context, BUTTON_B, 420, 125);
#endif



#if 0

    TradeNameReader reader(console, LANGUAGE, console);
    reader.read(feed.snapshot());


    InferenceBoxScope box(overlay, {0.920, 0.100, 0.020, 0.030});
    QImage image = extract_box(feed.snapshot(), box);
    ImageStats stats = image_stats(image);
    cout << stats.average << stats.stddev << endl;

//    is_white()


//    std::map<std::string, int> catch_count;
#endif




#if 0
    TradeStats stats;
    MultiConsoleErrorState error_state;

    env.run_in_parallel([&](ConsoleHandle& console){
        trade_current_pokemon(env, console, error_state, stats);
    });
#endif


//    trade_current_pokemon(env, console, error_state, stats);






#if 0
    QImage image("screenshot-20220213-141558364230.png");

//    CenterAButtonTracker tracker;
//    WhiteObjectWatcher detector(console, {0.40, 0.50, 0.40, 0.50}, {{tracker, false}});

//    detector.process_frame(image, current_time());

    ButtonDetector detector(console, console, ButtonType::ButtonA, {0.40, 0.50, 0.40, 0.50});


    AsyncVisualInferenceSession visual(env, console, console, console);
    visual += detector;
#endif




//    InferenceBoxScope box(overlay, 0.40, 0.50, 0.40, 0.50);


//    cout << WallClock::min() - current_time() << endl;

//    pbf_move_right_joystick(context, 0, 128, 45, 0);






#if 0
    InferenceBoxScope box(overlay, 0.905, 0.65, 0.08, 0.13);
    QImage image = extract_box(feed.snapshot(), box);

//    QImage image("test.png");
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        128, 255,
        128, 255,
        0, 255
    );
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20, false);
    cout << objects.size() << endl;


//    int c = 0;
//    for (const auto& object : objects){
//        extract_box(image, object).save("test-" + QString::number(c++) + ".png");
//    }

#if 1
    WaterfillObject object;
    for (const WaterfillObject& obj : objects){
        object.merge_assume_no_overlap(obj);
    }

    ImagePixelBox sbox(object.min_x, object.min_y, object.max_x, object.max_y);
    extract_box(image, sbox).save("test.png");
#endif

#endif




//    QImage image("2054071609223400_s.jpg");
//    SelectionArrowFinder arrow_detector(console, ImageFloatBox(0.350, 0.450, 0.500, 0.400));
//    arrow_detector.detect(image);



#if 0

//    ArcWatcher arcs(overlay);
//    BubbleWatcher bubbles(overlay);

    BubbleDetector bubbles;
    ArcDetector arcs;
    QuestMarkDetector quest_marks;
    WhiteObjectWatcher watcher(
        overlay,
        {{bubbles, false}, {arcs, false}, {quest_marks, false}}
    );
//    watcher.process_frame(feed.snapshot(), current_time());
#if 1
    {
        VisualInferenceSession session(env, logger, feed, overlay);
        session += watcher;
        session.run();
    }
#endif


#endif




#if 0
//    InferenceBoxScope box(overlay, 0.40, 0.50, 0.40, 0.50);
//    InferenceBoxScope box(overlay, 0.010, 0.700, 0.050, 0.100);

//    QImage image(feed.snapshot());
//    image = extract_box(image, box);

    QImage image("MountOn-Braviary-Original.png");

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        128, 255,
        128, 255,
        128, 255
    );
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20, false);
    cout << objects.size() << endl;

    int c = 0;
    for (const auto& object : objects){
        extract_box(image, object).save("test-" + QString::number(c++) + ".png");
    }
#endif




//    ShinySymbolWatcher watcher(overlay, SHINY_SYMBOL_BOX_BOTTOM);
//    watcher.process_frame(feed.snapshot(), current_time());


//    ArcDetector detector;


//    QImage image("screenshot-20220124-212851483502.png");
//    QImage image(feed.snapshot());
//    find_arcs(image);

#if 0
    QImage image0("test-image.png");
    QImage image1("test-sprite.png");
    for (int r = 0; r < image0.height(); r++){
        for (int c = 0; c < image0.width(); c++){
            uint32_t pixel1 = image1.pixel(c, r);
            if ((pixel1 >> 24) == 0){
                image0.setPixel(c, r, 0xff00ff00);
            }
        }
    }
    image0.save("test.png");
#endif



#if 0
    QImage image("screenshot-20220124-212851483502.png");

    InferenceBoxScope box(overlay, 0.4, 0.4, 0.2, 0.2);
    image = extract_box(image, box);
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        128, 255,
        128, 255,
        128, 255
    );
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20, false);
    cout << objects.size() << endl;

    int c = 0;
    for (const auto& object : objects){
        extract_box(image, object).save("test-" + QString::number(c++) + ".png");
    }

//    WaterfillObject object = objects[1];
//    object.merge_assume_no_overlap(objects[2]);

 //   extract_box(image, object).save("test.png");
#endif




#if 0
    QImage image("ArrowRight-Original.png");

    image = image.convertToFormat(QImage::Format::Format_ARGB32);
    uint32_t* ptr = (uint32_t*)image.bits();
    size_t words = image.bytesPerLine() / sizeof(uint32_t);
    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            uint32_t& pixel = ptr[r * words + c];
            uint32_t red = qRed(pixel);
            uint32_t green = qGreen(pixel);
            uint32_t blue = qBlue(pixel);
//            if (8 < c && c < 31 && 8 < r && r < 31){
//                continue;
//            }
            if (red < 0x80 || green < 0x80 || blue < 0x80){
                pixel = 0x00000000;
            }
        }
    }
    image.save("ArrowRight-Template.png");
#endif



#if 0
    QImage image("screenshot-20220123-225755803973.png");

    InferenceBoxScope box(overlay, 0.32, 0.87, 0.03, 0.04);
    image = extract_box(image, box);
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        128, 255,
        128, 255,
        128, 255
    );
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20, false);
    cout << objects.size() << endl;

//    int c = 0;
//    for (const auto& object : objects){
//        extract_box(image, object).save("test-" + QString::number(c++) + ".png");
//    }

    WaterfillObject object = objects[0];
    object.merge_assume_no_overlap(objects[1]);
    extract_box(image, object).save("Sparkle.png");

#endif





#if 0
    BlackBorderDetector detector;
    VideoOverlaySet overlays(overlay);
    detector.make_overlays(overlays);
    detector.detect(feed.snapshot());
#endif


#if 0
    ExperienceGainDetector detector;
    VideoOverlaySet overlays(overlay);
    detector.make_overlays(overlays);
    detector.detect(feed.snapshot());
#endif



#if 0
    ShinyEncounterTracker tracker(logger, overlay, BattleType::STANDARD);
    {
        VisualInferenceSession session(env, console, feed, overlay);
        session += tracker;
        session.run();
    }

    DoublesShinyDetection wild_result;
    ShinyDetectionResult your_result;
    determine_shiny_status(
        logger,
        wild_result, your_result,
        tracker.dialog_tracker(),
        tracker.sparkles_wild_overall(),
        tracker.sparkles_wild_left(),
        tracker.sparkles_wild_right(),
        tracker.sparkles_own()
    );
#if 0
    determine_shiny_status(
        logger,
        SHINY_BATTLE_REGULAR,
        tracker.dialog_timer(),
        tracker.sparkles_wild()
    );
#endif
#endif



#if 0
    VisualInferenceSession session(env, feed, overlay);
    ShinySparkleTracker tracker(overlay);
    session += tracker;
    session.run();
#endif








#if 0
    QImage image = feed.snapshot();
    PokemonSwSh::SparkleSet sparkles = PokemonSwSh::find_sparkles(image);

    VideoOverlaySet overlays(overlay);
    sparkles.draw_boxes(overlays, image, {0, 0, 1, 1});
#endif












    scope.wait_for(std::chrono::seconds(60));


}





}
}




