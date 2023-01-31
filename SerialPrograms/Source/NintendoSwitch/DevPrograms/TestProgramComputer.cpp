/*  Test Program (Computer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef _WIN64
#include <Windows.h>
#endif

#include <set>
#include <mutex>
#include <fstream>
#include <condition_variable>
#include <QImage>
#include <QJsonObject>
#include <QDir>
#include <QDateTime>
#include <QGuiApplication>
#include <QScreen>
#include <QPixmap>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/AlignedVector.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/Concurrency/AsyncDispatcher.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_Routines.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCodeReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "TestProgramComputer.h"
#include "ClientSource/Libraries/Logging.h"
#include "Common/Cpp/Containers/Pimpl.tpp"

#include "Kernels/BinaryMatrix/Kernels_PackedBinaryMatrixCore.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Core_64x4_Default.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "CommonFramework/Inference/ImageMatchDetector.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "CommonFramework/Tools/FileDownloader.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "Common/Cpp/Concurrency/PeriodicScheduler.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"
#include "Kernels/Kernels_Alignment.h"
#include "Kernels/ScaleInvariantMatrixMatch/Kernels_ScaleInvariantMatrixMatch.h"
#include "Kernels/SpikeConvolution/Kernels_SpikeConvolution.h"
#include "PokemonSwSh/MaxLair/AI/PokemonSwSh_MaxLair_AI.h"
#include "Kernels/AudioStreamConversion/AudioStreamConversion.h"
#include "Common/Cpp/StreamConverters.h"
#include "CommonFramework/AudioPipeline/AudioConstants.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"
#include "3rdParty/nlohmann/json.hpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageHSV32.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h"
#include "Common/Cpp/StringTools.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "Common/Cpp/EnumDatabase.h"
#include "PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterEnums.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h"

#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogArrowDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "Common/Cpp/Containers/BoxSet.h"
#include "Common/Cpp/Concurrency/ScheduledTaskRunner.h"
#include "PokemonSV/Inference/Battles/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"
#include "Integrations/DiscordWebhook.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_JoinTracker.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "CommonFramework/Environment/Environment.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/TimeQt.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichHandDetector.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Environment/Environment.h"

#ifdef PA_ARCH_x86
//#include "Kernels/Kernels_x64_SSE41.h"
//#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
// #include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_SSE42.h"
// #include "Kernels/Waterfill/Kernels_Waterfill_Core_64x8_x64_SSE42.h"
//#include "Kernels/Kernels_x64_SSE41.h"
//#include "Kernels/Kernels_x64_AVX2.h"
//#include "Kernels/Kernels_x64_AVX512.h"
//#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
//#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Intrinsics_x64_AVX512.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Core_64x32_x64_AVX512-GF.h"
//#include "Common/Cpp/CpuId/CpuId_x86.h"
//#include "Kernels/ImageScaling/Kernels_ImageScaling_Default.h"
//#include "Kernels/ImageScaling/Kernels_ImageScaling_x64_SSE41.h"
#endif

//#include <opencv2/core.hpp>


#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

using namespace Kernels;
using namespace Kernels::Waterfill;
using namespace Pokemon;


TestProgramComputer_Descriptor::TestProgramComputer_Descriptor()
    : ComputerProgramDescriptor(
        "Computer:TestProgram",
        "Computer", "Test Program (Computer)",
        "",
        "Test Program"
    )
{}
TestProgramComputer::TestProgramComputer()
    : STATIC_TEXT("test text")
    , SCREEN_WATCHER("Capture Box", 0, 0, 1, 1)
{
    PA_ADD_OPTION(STATIC_TEXT);
    PA_ADD_OPTION(SCREEN_WATCHER);
}

WallClock REFERENCE = current_time();


using namespace Kernels;

template <typename Type>
void print(const Type* ptr, size_t len){
    cout << "{";
    bool first = true;
    for (size_t c = 0; c < len; c++){
        if (!first){
            cout << ", ";
        }
        first = false;
        if (sizeof(Type) == 1){
            cout << (uint32_t)ptr[c];
        }else{
            cout << ptr[c];
        }
    }
    cout << "}" << endl;
}






void TestProgramComputer::program(ProgramEnvironment& env, CancellableScope& scope){
    using namespace Kernels;
//    using namespace NintendoSwitch::PokemonSwSh;
    using namespace NintendoSwitch::PokemonSV;
    using namespace Pokemon;
//    using namespace NintendoSwitch::PokemonSwSh::MaxLairInternal;

//    throw OperationFailedException(env.logger(), "asdf");
//    throw OperationFailedException(env.logger(), "asdf", std::make_shared<ImageRGB32>("20221118-024539201323.jpg"));
//    throw ProgramFinishedException();
//    throw FatalProgramException(env.logger(), "test");


//    send_program_telemetry(env.logger(), true, COLOR_RED, env.program_info(), "Test", {}, "");

//    throw ProgramFinishedException(env.logger(), "", std::make_shared<ImageRGB32>("TeraCode-S-chi-original.png"));


    ImageRGB32 image("screenshot-20230130-150721112888.png");
    SomethingInBoxSlotDetector detector(COLOR_RED);

    cout << detector.detect(image) << endl;


#if 0
    FILETIME idle_time, kernel_time, user_time;
    GetSystemTimes(&idle_time, &kernel_time, &user_time);
//    cout <<  << endl;
//    cout <<  << endl;
//    cout <<  << endl;

    uint64_t start_idle = idle_time.dwLowDateTime + ((uint64_t)idle_time.dwHighDateTime << 32);
    uint64_t start_kernel = kernel_time.dwLowDateTime + ((uint64_t)kernel_time.dwHighDateTime << 32);
    uint64_t start_user = user_time.dwLowDateTime + ((uint64_t)user_time.dwHighDateTime << 32);

    scope.wait_for(std::chrono::seconds(4));

    GetSystemTimes(&idle_time, &kernel_time, &user_time);
//    cout <<  << endl;
//    cout <<  << endl;
//    cout <<  << endl;

    uint64_t end_idle = idle_time.dwLowDateTime + ((uint64_t)idle_time.dwHighDateTime << 32);
    uint64_t end_kernel = kernel_time.dwLowDateTime + ((uint64_t)kernel_time.dwHighDateTime << 32);
    uint64_t end_user = user_time.dwLowDateTime + ((uint64_t)user_time.dwHighDateTime << 32);

    cout << (end_idle - start_idle) * 100 / 1000000000. << endl;
    cout << (end_kernel - start_kernel) * 100 / 1000000000. << endl;
    cout << (end_user - start_user) * 100 / 1000000000. << endl;
#endif

#if 0
    uint32_t src[4 * 4] = {
        0xff0a0a0a, 0xff141414, 0xff1e1e1e, 0xff282828,
        0xff646464, 0xff646464, 0xff646464, 0xff646464,
        0xff646464, 0xff646464, 0xff646464, 0xff646464,
        0xff646464, 0xff646464, 0xff646464, 0xff646464,
    };
    uint32_t dst[4 * 4] = {};

    print((uint8_t*)src + 0*16, 16);
    print((uint8_t*)src + 1*16, 16);
    print((uint8_t*)src + 2*16, 16);
    print((uint8_t*)src + 3*16, 16);

#if 0
    TileBuilder builder(
        dst, 3 * sizeof(uint32_t), 3, 3,
        src, 4 * sizeof(uint32_t), 4, 4
    );

    FloatPixelTile tile;
    size_t dst_col = 0;
    size_t src_col = 0;
    builder.build_tile(tile, dst_col, 0, src_col);
    cout << tile << endl;

    FloatPixelWord accumulator;
    size_t row = 0;
    builder.write_tile(accumulator, row, 0, tile, 0);
#endif

    scale_image_Default(
        dst, 4 * sizeof(uint32_t), 4, 4,
        src, 4 * sizeof(uint32_t), 4, 4
    );


    print((uint8_t*)dst + 0*16, 16);
    print((uint8_t*)dst + 1*16, 16);
    print((uint8_t*)dst + 2*16, 16);
    print((uint8_t*)dst + 3*16, 16);

//    print((uint8_t*)dst + 0*12, 12);
//    print((uint8_t*)dst + 1*12, 12);
//    print((uint8_t*)dst + 2*12, 12);


#if 0
    float dst[10];
    float src[6] = {1, 2, 3, 4, 5};


    scale_row(dst, 8, src, 5);
    print(dst, 8);
#endif
#endif


//    env.log("crash coming...", COLOR_RED);
//    cout << *(char*)nullptr << endl;


#if 0
    StatAccumulatorI32 stats;

    for (size_t c = 0; c < 1000; c++){
        scope.throw_if_cancelled();

        auto start = current_time();
        env.log(std::to_string(c));
        auto end = current_time();
        stats += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }
    cout << stats.dump("us", 1) << endl;
#endif


#if 0
    ImageRGB32 image("TeraCode-S-chi-original.png");
    for (size_t r = 0; r < image.height(); r++){
        for (size_t c = 0; c < image.width(); c++){
            uint32_t pixel = image.pixel(c, r);
            uint32_t sum = pixel & 0xff;
            sum += (pixel >> 16) & 0xff;
            sum += (pixel >> 8) & 0xff;
            if (sum > 400){
                image.pixel(c, r) = 0;
            }
        }
    }
    image.save("TeraCode-S-chi.png");
#endif



#if 0
    {
        ImageRGB32 image("TeraCodeTest-50.png");
        TeraLobbyReader reader;
        reader.raid_code(env.logger(), image);
    }
    {
        ImageRGB32 image("TeraCodeTest-S0.png");
        TeraLobbyReader reader;
        reader.raid_code(env.logger(), image);
    }
    {
        ImageRGB32 image("TeraCodeTest-S1.png");
        TeraLobbyReader reader;
        reader.raid_code(env.logger(), image);
    }
#endif






#if 0
    ImageRGB32 image("20230107-053814058280-FREESandwichHandNotDetected.png");

    SandwichHandLocator detector(SandwichHandType::FREE, {0, 0, 1, 1});
    auto ret = detector.detect(image);
    cout << ret.first << " " << ret.second << endl;
#endif



#if 0
//    cout << current_time() << endl;

    std::string str = to_utc_time_str(current_time());
    cout << str << endl;

    WallClock time = parse_utc_time_str(str);
    cout << to_utc_time_str(time) << endl;

//    parse_utc_time_str(str);
#endif

#if 0
    ThreadHandle handle = current_thread_handle();
    cout << thread_cpu_time(handle).count() << endl;

    WallClock start = current_time();
    while (current_time() - start < std::chrono::seconds(1));

    cout << thread_cpu_time(handle).count() << endl;
#endif


#if 0
    ImageRGB32 image("20221230-232826566125-BoxSystemNotDetected.png");
    BoxDetector detector;
//    cout << detector.detect(image) << endl;

    auto ret = detector.detect_location(image);
    cout << (int)ret.first << " : " << (int)ret.second.row << ", " << (int)ret.second.col << endl;

    GradientArrowDetector arrow(COLOR_RED, GradientArrowType::DOWN, {0.240, 0.160, 0.380, 0.550});
    cout << arrow.detect(image) << endl;
#endif


#if 0
    ImageRGB32 image("20221230-075353892425-BoxSystemNotDetected.png");
    BoxDetector detector;
//    cout << detector.detect(image) << endl;

    auto ret = detector.detect_location(image);
    cout << (int)ret.first << " : " << (int)ret.second.row << ", " << (int)ret.second.col << endl;

    GradientArrowDetector arrow(COLOR_RED, GradientArrowType::DOWN, {0.140, 0.150, 0.050, 0.700});
    cout << arrow.detect(image) << endl;
#endif



#if 0
    ImageRGB32 image("ZeroGateNightBike_2_True.png");
    OverworldDetector detector;
    cout << detector.detect(image) << endl;
#endif

#if 0
    ImageRGB32 image("screenshot-20221219-201912436404.png");
    PromptDialogDetector prompt(COLOR_RED, {0.623, 0.530, 0.243, 0.119});
    cout << prompt.detect(image) << endl;
#endif


#if 0
    ImageRGB32 image("20221219-140347621956.jpg");
    TeraLobbyReader reader;
    auto names = reader.read_names(env.logger(), {Language::English}, true, image);
#endif
#if 0
    for (size_t row = 0; row < image.height(); row++){
        for (size_t col = 0; col < image.width(); col++){
            uint32_t pixel = image.pixel(col, row);
            uint32_t r = (pixel >> 16) & 0xff;
            uint32_t g = (pixel >>  8) & 0xff;
            uint32_t b = (pixel >>  0) & 0xff;
#if 0
            if (b > 2*r + 10 || b > 2*g + 10){
                pixel = 0xffffffff;
            }
#endif
#if 1
            if (r + g + b < 150){
                pixel = 0xffffffff;
            }
#endif
            image.pixel(col, row) = pixel;
        }
    }
    image.save("test.png");
#endif


#if 0
    MultiLanguageJoinTracker tracker;
    tracker.add(Language::English, "Alice", "RNS308");
    tracker.add(Language::English, "Alice", "RNS308");
    tracker.add(Language::English, "Alice", "RNS308");
    tracker.add(Language::English, "Dhruv", "UCCJ9H");
    tracker.add(Language::English, "Dhruv", "EKNQY9");
    tracker.add(Language::English, "Gael", "X89986");
    tracker.add(Language::English, "Gael", "X89986");
    tracker.add(Language::ChineseSimplified, "Gael", "X89986");
    tracker.add(Language::ChineseSimplified, "Gael", "X89986");

    tracker.dump("test.txt");
#endif

#if 0
    cout << tracker.dump() << endl;

    std::ofstream file("test.txt");
    file << tracker.dump();
    file.close();
#endif

#if 0
    Integration::DiscordWebhook::send_message(
        env.logger(), false,
        {"Notifs"},
        "Test Join Tracker output...",
        JsonArray(),
        std::make_shared<PendingFileSend>("test.txt", false)
    );
#endif


#if 0
    ImageRGB32 image("BadArrow.png");
    TeraCatchDetector detector(COLOR_RED);
    cout << detector.detect(image) << endl;


//    extract_box_reference(image, ImageFloatBox{0.95, 0.81, 0.02, 0.06}).save("test.png");
#endif


#if 0
    {
        std::string json = FileDownloader::download_file(env.logger(), "https://raw.githubusercontent.com/PokemonAutomation/ServerConfigs-PA-SHA/main/PokemonScarletViolet/TeraAutoHost-BanList.json");
        cout << "Done loading..." << endl;

        std::ofstream file("test.txt");
        file << json;
    }
#if 1
    {
        JsonValue json = FileDownloader::download_json_file(env.logger(), "https://raw.githubusercontent.com/PokemonAutomation/ServerConfigs-PA-SHA/main/PokemonScarletViolet/TeraAutoHost-BanList.json");
        cout << "Done loading..." << endl;
        cout << json.dump() << endl;
    }
#endif
#endif

#if 0
    ScheduledTaskRunner scheduler(env.inference_dispatcher());

    scheduler.add_event(std::chrono::seconds(5), []{ cout << "5 seconds" << endl; });
    scheduler.add_event(std::chrono::seconds(2), []{ cout << "2 seconds" << endl; });
    scheduler.add_event(std::chrono::seconds(7), []{ cout << "7 seconds" << endl; });

    scope.wait_for(std::chrono::seconds(100));
#endif

#if 0
    BoxSet<size_t> set;
    cout << set.dump() << endl;

    set.insert({10, 20, 15, 25});
    set.insert({11, 19, 13, 27});
    set.insert({12, 18, 13, 27});
    cout << set.dump() << endl;

    auto iter = set.lower_bound_min_y(11);
    for (; iter != set.end_min_y(); ++iter){
        cout << iter->first << endl;
    }
#endif

//    ImageRGB32 image("SV-Buttons2.png");
//    WhiteButtonDetector detector(WhiteButton::ButtonB, ImageFloatBox{0, 0, 1, 1});
//    detector.detect(image);


#if 0
    ImageRGB32 image("SV-Buttons2.png");

    ImageFloatBox box(0.027, 0.922, .02, .035);
    ImageViewRGB32 region = extract_box_reference(image, box);

    region.save("tmp.png");

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(region, 0xff808080, 0xffffffff);
    auto objects = Waterfill::find_objects_inplace(matrix, 20);
    extract_box_reference(region, objects[0]).save("cropped.png");
#endif

//    DialogArrowDetector detector({0.45, 0.9, .1, .05});
//    detector.detect(image);

#if 0
    ImageFloatBox box(0.72, 0.85, 0.02, 0.035);

    ImageViewRGB32 region = extract_box_reference(image, box);

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(region, 0xff000000, 0xff7f7fbf);

    cout << matrix.dump() << endl;

    auto arrow = Waterfill::find_objects_inplace(matrix, 20);
    extract_box_reference(region, arrow[0]).save("cropped.png");
#endif

#if 0
    ImageRGB32 image("SV-BattleMenu.png");
    image.scale_to(1920, 1080);

    GradientArrowDetector detector({0.75, 0.63, 0.05, 0.1});
    detector.detect(image);
#endif


#if 0
    ImageRGB32 image("Arrow-Alpha.png");
    for (size_t r = 0; r < image.height(); r++){
        for (size_t c = 0; c < image.width(); c++){
            uint32_t& pixel = image.pixel(c, r);
            if (pixel == 0xffffffff){
                pixel = 0;
            }
        }
    }
    image.save("Arrow-Alpha2.png");
#endif




#if 0
    ImageRGB32 image("SV-BattleMenu.png");
    image.scale_to(1920, 1080);

    ImageViewRGB32 box = extract_box_reference(image, ImageFloatBox({0.75, 0.63, 0.05, 0.1}));


    PackedBinaryMatrix yellow = compress_rgb32_to_binary_range(box, 0xff808000, 0xffffff7f);
    PackedBinaryMatrix blue = compress_rgb32_to_binary_range(box, 0xff004080, 0xff8fffff);

    cout << yellow.dump() << endl;
    cout << blue.dump() << endl;

//    auto session = make_WaterfillSession(yellow);
//    session.

    auto yellows = Waterfill::find_objects_inplace(yellow, 100);
    auto blues = Waterfill::find_objects_inplace(blue, 100);
    cout << yellows.size() << endl;
    cout << blues.size() << endl;

    size_t c = 0;
    for (auto& item : yellows){
        extract_box_reference(box, item).save("yellow-" + std::to_string(c++) + ".png");
    }
    c = 0;
    for (auto& item : blues){
        extract_box_reference(box, item).save("blue-" + std::to_string(c++) + ".png");
    }

    Waterfill::WaterfillObject obj = yellows[1];
    obj.merge_assume_no_overlap(blues[0]);

    extract_box_reference(box, obj).save("Arrow.png");
#endif


//    cout << "\u274c\u2705" << endl;


//    SummaryShinySymbolDetector detector;


#if 0
    BattleMenuDetector detector;
    cout << detector.detect(image) << endl;
#endif

#if 0
    ImageRGB32 image("screenshot-20220814-144223026390.png");

    StandardBattleMenuDetector detector(false);
    cout << detector.detect(image) << endl;
#endif


//    STATIC_TEXT.set_text("123456789");



//    cv::Mat image;


//    env.log(QString(QChar(0x2728)));
//    env.log("\u2728\u2733");


//    throw UserSetupError(env.logger(), "Can't find any shinies? Join our Discord server and DM Elvis for FREE SHINIES!");



//    ImageRGB32 image("20220806-151454864146-rmsd_precropped_input.png");

//    image = filter_rgb32_range(image, 0xffb0b0b0, 0xffffffff, Color(0xffff0000), true);

//    image.save("test.png");


//    cout << StringTools::replace("asdf asdf adsf", "123", "---") << endl;



#if 0
    ImageRGB32 image("screenshot-20220725-170822724101.png");
    ImageHSV32 hsv(image);
#endif


#if 0
    ImageRGB32 image(100, 100);
    image.fill(0xffff0000);
    image.save("test.png");
#endif

#if 0
    ImageRGB32 image0("Avermedia-Qt5.png");
    ImageRGB32 image1(std::move(image0));

    image1.sub_image(100, 100, 100, 100).save("test.png");
#endif


#if 0
//    QImage image("20220714-114859147833-connect_to_internet_with_inference.png");
//    QImage image("MyPin-Qt6.png");

    YCommMenuDetector detector(true);
    cout << detector.detect(QImage("MiraBox-Qt5.png")) << endl;
    cout << detector.detect(QImage("MiraBox-Qt5.png")) << endl;
    cout << detector.detect(QImage("MyPin-Qt5.png")) << endl;
    cout << detector.detect(QImage("MyPin-Qt6.png")) << endl;
    cout << detector.detect(QImage("NoBrand-Qt6.png")) << endl;
    cout << detector.detect(QImage("ShadowCast-Qt6.png")) << endl;
    #endif



#if 0
    GlobalState state;
    state.boss = "dialga";
    state.players[0].pokemon = "cradily";
    state.players[1].console_id = 0;
    state.players[1].pokemon = "heatmor";
    state.players[2].pokemon = "crawdaunt";
    state.players[3].pokemon = "marowak-alola";
    state.opponent = {"flareon"};


    select_move(env.logger(), state, 1);
#endif

#if 0
    using namespace nlohmann;

    json j2 = {
      {"pi", 3.141},
      {"happy", true},
      {"name", "Niels"},
      {"nothing", nullptr},
      {"answer", {
        {"everything", 42}
      }},
      {"list", {1, 0, 2}},
      {"object", {
        {"currency", "USD"},
        {"value", 42.99}
      }}
    };

//    cout << "str = " << 123 << endl;
//    std::string str = j2.get<std::string>();
//    cout << "str = " << str << endl;

    cout << json((int8_t)1).dump() << endl;
    cout << json((int16_t)1).dump() << endl;
    cout << json((int32_t)1).dump() << endl;
    cout << json((int64_t)1).dump() << endl;

    {
        cout << j2.dump() << endl;
        JsonValue2 value = from_nlohmann(j2);
        json j3 = to_nlohmann(value);
        cout << j3.dump() << endl;
    }
    QJsonObject obj = QJsonDocument::fromJson(j2.dump().c_str()).object();
    {
        cout << j2.dump() << endl;
        JsonValue2 value = from_QJson(obj);
        QJsonValue obj2 = to_QJson(value);
//        cout << QJson_to_nlohmann(obj2).dump() << endl;
    }

//    cout << QDir::current().relativeFilePath(RESOURCE_PATH()).toStdString() << endl;
#endif

#if 0
    std::string str;


    cout << j2.dump(4) << endl;

    json j3 = "asdf";
    cout << j3.dump(4) << endl;

    QJsonDocument doc(QJsonDocument::fromJson("asdf"));
    cout << doc.toJson().data() << endl;
#endif

#if 0
    int16_t in[4] = {1, 1, 2, -2};
    print_u8((uint8_t*)in, 8);



    AudioSourceReader reader(AudioSampleFormat::SINT16, 1, false);
    AudioListener listener(1);
    reader += listener;


//    reader.push_bytes(in, 8);
    reader.push_bytes((char*)in + 0, 5);
    reader.push_bytes((char*)in + 5, 3);
#endif



//    AudioStreamReader2 reader(2, AudioSampleFormat::SINT16);



#if 0
    char buffer[17] = {};
    for (size_t c = 0; c < 16; c++){
        buffer[c] = '=';
    }

    StreamReader reader(8);
    cout << reader.dump() << endl;

    reader.push_back("asdf", 4);
    reader.pop_to(2);
    reader.push_back("qwerzx", 6);
    cout << reader.dump() << endl;

    reader.push_back("sdfg", 4);
    cout << reader.dump() << endl;

//    reader.read(buffer, 4, 7);
//    cout << buffer << endl;
//    cout << reader.dump() << endl;
#endif

#if 0
    float f[10];
    uint8_t i[10] = {};
    i[0] = 1;
    i[1] = 255;
    i[2] = 0;
    i[3] = 123;
    i[9] = 123;

    Kernels::AudioStreamConversion::convert_audio_uint8_to_float(f, i, 10);
    print(f, 10);
    memset(i, 0, sizeof(i));
    Kernels::AudioStreamConversion::convert_audio_float_to_uint8(i, f, 10);
    print_u8(i, 10);
#endif


#if 0
    union{
        float f32;
        int32_t i32;
    };
    f32 = 12582912.;
    cout << i32 << endl;
#endif


//    cout << _mm_cvt_ss2si() << endl;

#if 0
    CircularBuffer buffer(8);

    char data[21] = {};

    buffer.push_back("asdfqwer", 8);

    cout << buffer.pop_front(data, 4) << endl;
    cout << data << endl;

//    cout << buffer.dump() << endl;

    buffer.push_back("zxcvsdfg", 8);

    cout << buffer.pop_front(data, 20) << endl;
    cout << data << endl;
#endif


//    __m256 k0 = _mm256_set1_ps(-4.);
//    __m256 k1 = _mm256_set1_ps(-3.);
//    __m256 k2 = _mm256_set1_ps(-2.);
//    __m256 k3 = _mm256_set1_ps(-1.);






}





inline std::string dump8(uint8_t x){
    std::string str;
    for (size_t c = 0; c < 8; c++){
        str += ((x >> c) & 1) ? "1" : "0";
    }
    return str;
}


void print(const uint64_t* ptr, size_t len){
    cout << "{";
    bool first = true;
    for (size_t c = 0; c < len; c++){
        if (!first){
            cout << ", ";
        }
        first = false;
        cout << ptr[c];
    }
    cout << "}" << endl;
}









}
