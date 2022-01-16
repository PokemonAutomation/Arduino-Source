/*  Test Program (Switch)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
//#include <QSystemTrayIcon>
#include <QProcess>
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/AlignedVector.h"
#include "Common/Cpp/SIMDDebuggers.h"
#include "Common/Qt/QtJsonTools.h"
#include "ClientSource/Libraries/Logging.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/StatsDatabase.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "CommonFramework/Inference/AnomalyDetector.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CommonFramework/Inference/TimeWindowStatTracker.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "CommonFramework/Inference/FrozenImageDetector.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageMatch/FilterToAlpha.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/ImageTools/CommonFilters.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/ImageTools/FillGeometry.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_Filtering.h"
#include "CommonFramework/OCR/OCR_StringNormalization.h"
#include "CommonFramework/OCR/OCR_TextMatcher.h"
#include "CommonFramework/OCR/OCR_LargeDictionaryMatcher.h"
#include "CommonFramework/ImageMatch/ExactImageDictionaryMatcher.h"
#include "CommonFramework/ImageMatch/CroppedImageDictionaryMatcher.h"
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokeballSprites.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyFilters.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SparkleTrigger.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SquareTrigger.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SquareDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyTrigger.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinySparkleDetector.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidLobbyReader.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Inference/PokemonSwSh_FishingDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "PokemonSwSh/Inference/PokemonSwSh_ReceivePokemonDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_PokemonSpriteReader.h"
#include "PokemonSwSh/Inference/PokemonSwSh_TypeSymbolFinder.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleBallReader.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_ExperienceGainDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Entrance.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ItemSelectMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ProfessorSwap.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_CaughtScreen.h"
#include "PokemonSwSh/MaxLair/Program/PokemonSwSh_MaxLair_Run_Entrance.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_PathMatchup.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI_RentalBossMatchup.h"
#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Moves.h"
#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Pokemon.h"
#include "PokemonSwSh/PkmnLib/PokemonSwSh_PkmnLib_Matchup.h"
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh/Programs/PokemonSwSh_Internet.h"
#include "PokemonSwSh/Resources/PokemonSwSh_TypeSprites.h"
#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
//#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
#include "Kernels/ImageStats/Kernels_ImagePixelSumSqr.h"
#include "Kernels/ImageStats/Kernels_ImagePixelSumSqrDev.h"
#include "Kernels/Kernels_Alignment.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Intrinsics_SSE4.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_FillQueue.h"
//#include "Kernels/BinaryImage/Kernels_BinaryImage_Default.h"
//#include "Kernels/BinaryImage/Kernels_BinaryImage_x64_SSE42.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Default.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_SSE42.h"
//#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_AVX2.h"
//#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_AVX512.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "Integrations/DiscordWebhook.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyDialogTracker.h"
#include "PokemonBDSP/PokemonBDSP_Settings.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "CommonFramework/ImageTools/ColorClustering.h"
#include "PokemonBDSP/Inference/PokemonBDSP_DialogDetector.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyEncounterDetector.h"
#include "PokemonBDSP/Inference/ShinyDetection/PokemonBDSP_ShinyTrigger.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MarkFinder.h"
#include "PokemonBDSP/Programs/PokemonBDSP_GameEntry.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MapDetector.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleBallReader.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleMenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_VSSeekerReaction.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_StartBattleDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxDetector.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_BoxShinyDetector.h"
#include "PokemonBDSP/Programs/Eggs/PokemonBDSP_EggRoutines.h"
#include "PokemonBDSP/Programs/Eggs/PokemonBDSP_EggFeedback.h"
#include "PokemonBDSP/Programs/PokemonBDSP_RunFromBattle.h"
#include "PokemonBDSP/Programs/PokemonBDSP_BoxRelease.h"
#include "PokemonBDSP/Inference/BoxSystem/PokemonBDSP_IVCheckerReader.h"
//#include "CommonFramework/BinaryImage/BinaryImage.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSide.h"
#include "PokemonSwSh/Inference/PokemonSwSh_TypeSymbolFinder.h"
#include "TestProgramSwitch.h"

#include <immintrin.h>
#include <fstream>
#include <QHttpMultiPart>
#include <QFile>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

//#include <Windows.h>
#include <iostream>
using std::cout;
using std::endl;


//#include "../Internal/SerialPrograms/NintendoSwitch_Commands_ScalarButtons.h"




namespace PokemonAutomation{
namespace NintendoSwitch{


TestProgram_Descriptor::TestProgram_Descriptor()
    : MultiSwitchProgramDescriptor(
        "NintendoSwitch:TestProgram",
        "Nintendo Switch", "Test Program (Switch)",
        "",
        "Test Program (Switch)",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB,
        1, 4, 1
    )
{}


TestProgram::TestProgram(const TestProgram_Descriptor& descriptor)
    : MultiSwitchProgramInstance(descriptor)
    , LANGUAGE(
        "<b>OCR Language:</b>",
        { Language::English }
    )
//    , TABLE({
//        {"Description", {true, true, false, ImageAttachmentMode::JPG, {"Notifs", "Showcase"}, std::chrono::seconds(60)}},
//    })
{
    PA_ADD_OPTION(LANGUAGE);
//    PA_ADD_OPTION(TABLE);
}


//using namespace Kernels;
//using namespace Kernels::Waterfill;



namespace PokemonBDSP{


}








void TestProgram::program(MultiSwitchProgramEnvironment& env){
    using namespace Kernels;
    using namespace Kernels::Waterfill;
    using namespace OCR;
    using namespace Pokemon;
    using namespace PokemonBDSP;

    Logger& logger = env.logger();
    ConsoleHandle& console = env.consoles[0];
    BotBase& botbase = env.consoles[0];
    VideoFeed& feed = env.consoles[0];
    VideoOverlay& overlay = env.consoles[0];


//    PokemonSwSh::SelectionArrowFinder finder(overlay, ImageFloatBox(0.640, 0.600, 0.055, 0.380));

//    QImage image("screenshot-20220108-185053570093.png");
//    cout << finder.detect(image) << endl;

//    QImage image("20220111-124433054843-PathPartyReader-ReadHP.png");
//    QImage image("20220116-044701249467-ReadPathSide.png");

//    cout << (int)PokemonSwSh::MaxLairInternal::read_side(image) << endl;

#if 1
    QImage image("20220116-053836954926-ReadPath.png");

    std::multimap<double, std::pair<PokemonType, ImagePixelBox>> candidates = PokemonSwSh::find_symbols(image, 0.20);


//    std::deque<InferenceBoxScope> hits;
//    hits.clear();
    cout << "---------------" << endl;
    for (const auto& item : candidates){
        cout << get_type_slug(item.second.first) << ": " << item.first << endl;
//        hits.emplace_back(overlay, translate_to_parent(screen, box, item.second.second), COLOR_GREEN);
    }
#endif


#if 0
    uint16_t FLY_A_TO_X_DELAY = 50;


    pbf_press_button(console, BUTTON_R, 5, 0);
    pbf_press_dpad(console, DPAD_RIGHT, 10, 115);
    pbf_press_button(console, BUTTON_ZL, 10, 0);
    console.botbase().wait_for_all_requests();
    MapWatcher detector;
    int ret = wait_until(
        env, console, std::chrono::seconds(2),
        { &detector }
    );
    if (ret < 0){
        console.log("Map not detected after 2 seconds.", COLOR_RED);
//        PA_THROW_StringException("Map not detected after 2 seconds.");
    }else{
        console.log("Detected map!", COLOR_BLUE);
    }

    env.wait_for(std::chrono::seconds(1));

    //  Move bolder and cursor to Celestial town.
    pbf_press_dpad(console, DPAD_RIGHT, 30, 95);

    //  Bring up menu
    pbf_press_button(console, BUTTON_ZL, 20, FLY_A_TO_X_DELAY - 20);
    pbf_press_button(console, BUTTON_X, 20, GameSettings::instance().OVERWORLD_TO_MENU_DELAY);

    //  Fly
    pbf_press_button(console, BUTTON_ZL, 20, 10 * TICKS_PER_SECOND);

#if 0
    //  Enter Pokemon.
    pbf_press_button(console, BUTTON_ZL, 20, GameSettings::instance().MENU_TO_POKEMON_DELAY);

    //  Enter Summary.
    pbf_press_button(console, BUTTON_ZL, 20, 105);
    pbf_press_button(console, BUTTON_ZL, 20, 5 * TICKS_PER_SECOND);

    //  Local Room
    pbf_press_button(console, BUTTON_Y, 20, 105);
#endif
#endif




//    QImage image("20220111-124433054843-PathPartyReader-ReadHP.png");

//    NintendoSwitch::PokemonSwSh::MaxLairInternal::PathReader detector(overlay, 0);
//    double hp[4];
//    detector.read_hp(logger, image, hp);



#if 0
    MapDetector detector;
    VideoOverlaySet set(overlay);
    detector.make_overlays(set);

    cout << detector.detect(feed.snapshot()) << endl;
#endif



//    QImage image("screenshot-20220108-185053570093.png");
//    PokemonSwSh::MaxLairInternal::BattleMenuReader reader(overlay, Language::English);

//    cout << reader.can_dmax(image) << endl;




//    SelectionArrowFinder detector(overlay, {0.50, 0.58, 0.40, 0.10}, COLOR_RED);
//    detector.detect(feed.snapshot());



//    InferenceBoxScope box(overlay, {0.23, 0.30, 0.35, 0.30});


#if 0
    QImage image("screenshot-20220103-011451179122.png");

    PackedBinaryMatrix matrix = filter_rgb32_range(
        image,
        192, 255,
        0, 160,
        0, 192
    );

    std::vector<WaterFillObject> objects = find_objects(matrix, 100, false);
    VideoOverlaySet set(overlay);
    size_t c = 0;
    for (const WaterFillObject& object : objects){
        ImagePixelBox box(object.min_x, object.min_y, object.max_x, object.max_y);
        ImageFloatBox fbox = translate_to_parent(image, {0, 0, 1, 1}, box);
        set.add(COLOR_RED, fbox);

        image.copy(object.min_x, object.min_y, object.width(), object.height()).save("test-" + QString::number(c++) + ".png");
    }
#endif


#if 0
    QImage image("ExclamationTop-0.png");

    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            uint32_t pixel = image.pixel(c, r);
            cout << "(" << qRed(pixel) << "," << qGreen(pixel) << "," << qBlue(pixel) << ")";
        }
        cout << endl;
    }
#endif



#if 0
    QImage image("QuestionTop-0.png");
    image = image.convertToFormat(QImage::Format_ARGB32);

    uint32_t* ptr = (uint32_t*)image.bits();
    size_t words = image.bytesPerLine() / sizeof(uint32_t);

    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            uint32_t pixel = ptr[r*words + c];
            if (qRed(pixel) + qGreen(pixel) + qBlue(pixel) < 50){
                ptr[r*words + c] = 0;
            }
        }
    }

    image.save("QuestionTop-1.png");
#endif


#if 0
    cout << std::hex << QColor("green").rgb() << endl;
    cout << std::hex << QColor(Qt::green).rgb() << endl;
    cout << std::hex << QColor(Qt::darkGreen).rgb() << endl;
    cout << std::hex << QColor(Qt::darkCyan).rgb() << endl;
#endif


#if 0
    QImage image("screenshot-20211227-082121670685.png");
    image = extract_box(image, ImageFloatBox({0.95, 0.10, 0.05, 0.10}));
    image.save("test.png");


    BinaryImage binary_image = filter_rgb32_range(
        image,
        255, 255,
        128, 255,
        0, 128,
        0, 128
    );
    cout << binary_image.dump() << endl;
#endif


#if 0
    ShortDialogDetector detector;
    OverlaySet overlays(overlay);
    detector.make_overlays(overlays);

    cout << detector.detect(QImage("20211228-013942613330.jpg")) << endl;
//    cout << detector.detect(feed.snapshot()) << endl;
#endif



#if 0
    BoxShinyDetector detector;
    cout << detector.detect(QImage("20211226-031611120900.jpg")) << endl;

//    pbf_mash_button(console, BUTTON_X, 10 * TICKS_PER_SECOND);
#endif


#if 0
    BattleMenuDetector detector(BattleType::WILD);
    OverlaySet overlays(overlay);
    detector.make_overlays(overlays);
#endif


    env.wait_for(std::chrono::seconds(60));


}





}
}




