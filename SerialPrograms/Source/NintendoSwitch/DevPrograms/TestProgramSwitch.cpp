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
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"


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
    using namespace NintendoSwitch;
    using namespace Pokemon;
    using namespace PokemonSwSh;
//    using namespace PokemonBDSP;
//    using namespace PokemonLA;
//    using namespace PokemonSV;

    [[maybe_unused]] Logger& logger = env.logger();
    [[maybe_unused]] ConsoleHandle& console = env.consoles[0];
//    [[maybe_unused]] BotBase& botbase = env.consoles[0];
    [[maybe_unused]] VideoFeed& feed = env.consoles[0];
    [[maybe_unused]] VideoOverlay& overlay = env.consoles[0];



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
    ImageRGB32 image("GermanUpdate2.png");
//    auto image = feed.snapshot();
//    HomeDetector detector;
    UpdateMenuDetector detector;
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
 //   dialog_arrow_detector.make_overlays(set);
 //   gradient_arrow_detector.make_overlays(set);
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





}
}




