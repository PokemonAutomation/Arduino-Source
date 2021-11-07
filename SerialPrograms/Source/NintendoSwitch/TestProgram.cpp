/*  Test Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
//#include <QSystemTrayIcon>
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
#include "CommonFramework/Inference/FrozenImageDetector.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageMatch/FilterToAlpha.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/ImageTools/CommonFilters.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_Filtering.h"
#include "CommonFramework/OCR/OCR_StringNormalization.h"
#include "CommonFramework/OCR/OCR_TextMatcher.h"
#include "CommonFramework/OCR/OCR_LargeDictionaryMatcher.h"
#include "CommonFramework/ImageMatch/ExactImageDictionaryMatcher.h"
#include "CommonFramework/ImageMatch/CroppedImageDictionaryMatcher.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
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
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogDetector.h"
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
#include "Kernels/WaterFill/Kernels_WaterFill_Intrinsics_SSE4.h"
#include "Kernels/WaterFill/Kernels_WaterFill_FillQueue.h"
#include "Kernels/BinaryImage/Kernels_BinaryImage_Default.h"
#include "Kernels/BinaryImage/Kernels_BinaryImage_x64_SSE42.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_Default.h"
#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_SSE42.h"
//#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_AVX2.h"
//#include "Kernels/BinaryImageFilters/Kernels_BinaryImage_BasicFilters_x64_AVX512.h"
#include "Integrations/DiscordWebHook.h"
#include "Pokemon/Pokemon_Notification.h"
#include "TestProgram.h"

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


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


double relative_damage(const MaxLairMon& you, const MaxLairMon& opponent, uint8_t move_slot, bool dmax);


}
}
}
}




namespace PokemonAutomation{
namespace NintendoSwitch{


TestProgram_Descriptor::TestProgram_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "NintendoSwitch:TestProgram",
        "Test Program",
        "",
        "Test Program",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


TestProgram::TestProgram(const TestProgram_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE(
        "<b>OCR Language:</b>",
        m_iv_checker_reader.languages()
    )
//    , TABLE({
//        {"Description", {true, true, false, ImageAttachmentMode::JPG, {"Notifs", "Showcase"}, std::chrono::seconds(60)}},
//    })
{
    PA_ADD_OPTION(LANGUAGE);
//    PA_ADD_OPTION(TABLE);
}


using namespace Kernels;
using namespace Kernels::WaterFill;





void TestProgram::program(SingleSwitchProgramEnvironment& env){
    using namespace OCR;
    using namespace Pokemon;
    using namespace PokemonSwSh::MaxLairInternal;
    using namespace papkmnlib;

    BotBase& botbase = env.console;
    VideoFeed& feed = env.console;
    VideoOverlay& overlay = env.console;


#if 0
    PokemonCaughtMenuDetector detector;

    QImage image("20211106-143106513770-ProgramHang.png");
    cout << detector.detect(image) << endl;
#endif

#if 0
    InferenceBoxScope box(env.console, 0.945, 0.010, 0.0525, 0.050);

    QImage image = extract_box(feed.snapshot(), box);
    OCR::TextImageFilter{false, 600}.apply(image);
    image.save("test.png");
#endif



//    InferenceBoxScope box0(overlay, 0.760, 0.400 + 0*0.133, 0.050, 0.030, Qt::blue);
//    InferenceBoxScope box1(overlay, 0.760, 0.400 + 1*0.133, 0.050, 0.030, Qt::blue);
//    InferenceBoxScope box2(overlay, 0.760, 0.400 + 2*0.133, 0.050, 0.030, Qt::blue);
//    InferenceBoxScope box3(overlay, 0.760, 0.400 + 3*0.133, 0.050, 0.030, Qt::blue);


//    cout << MaxLairInternal::count_catches(overlay, QImage("test.png")) << endl;


//    SelectionArrowFinder arrow_detector(env.console, ImageFloatBox(0.450, 0.450, 0.400, 0.400));
//    AsyncVisualInferenceSession session(env, env.console, env.console);
//    session += arrow_detector;

#if 0
    QImage screen = env.console.video().snapshot();

    InferenceBoxScope box0(env.console, 0.180, 0.815, 0.015, 0.030);
    InferenceBoxScope box1(env.console, 0.785, 0.840, 0.030, 0.050);

    ImageStats stats0 = image_stats(extract_box(screen, box0));
    ImageStats stats1 = image_stats(extract_box(screen, box1));
    cout << stats0.average << stats0.stddev << endl;
    cout << stats1.average << stats1.stddev << endl;
#endif

#if 0
    InferenceBoxScope box(env.console, 0.900, 0.015, 0.020, 0.040);

    ImageStats stats = image_stats(extract_box(feed.snapshot(), box));
    cout << stats.average << endl;
    cout << stats.stddev << endl;
#endif


#if 0
    GlobalState state;
    state.boss = "palkia";
    state.opponent = {"azumarill"};
    state.opponent_hp = 0.79;
//    state.players[1].dmax_turns_left = 0;
    state.players[0].console_id = 0;
    state.players[1].console_id = 1;
    state.players[2].console_id = -1;
    state.players[3].console_id = -1;
    state.players[0].pokemon = "lilligant";
    state.players[1].pokemon = "linoone-galar";
    state.players[2].pokemon = "charmeleon";
    state.players[3].pokemon = "shiinotic";

    select_move(env.logger(), state, 0);
#endif


#if 0
    const Pokemon& boss = get_pokemon("zygarde-50");

    std::multimap<double, std::string, std::greater<double>> rank;
    for (const auto& rental : all_rental_pokemon()){
//        rank.emplace(evaluate_matchup(rental.second, boss, {}, 4), rental.first);
        rank.emplace(rental_vs_boss_matchup(rental.first, boss.name()), rental.first);
    }

    for (const auto& item : rank){
        cout << item.first << " : " << item.second <<endl;
    }
#endif


#if 0
    GlobalState state;
    state.players[0].console_id = 0;
    state.players[1].console_id = 1;
    state.players[2].console_id = 2;
    state.players[3].console_id = 3;
    state.players[0].pokemon = "hatterene";
    state.players[1].pokemon = "drifblim";
    state.players[2].pokemon = "ivysaur";
    state.players[3].pokemon = "alcremie";

    should_swap_with_professor(env.logger(), state, 0);
#endif


//    cout << type_vs_boss(PokemonType::NORMAL, "lugia") << endl;


#if 0
    for (const auto& item : papkmnlib::all_boss_pokemon()){
//        cout << item.first << ": " << item.second.ability() << endl;
        item.second.mo
    }
    for (const auto& item : papkmnlib::all_rental_pokemon()){
//        cout << item.first << ": " << item.second.ability() << endl;
    }
#endif



//    for (const auto& item : papkmnlib::get_all_moves()){
//        cout << item.first << ": " << item.second.name() << endl;
//    }



#if 0
    bool on = false;
    while (true){
        on = !on;
        set_leds(env.console, on);
        pbf_wait(env.console, 20);
    }
#endif


#if 0
    EventNotificationOption event("", true, true, ImageAttachmentMode::JPG);

    send_program_notification(
        env.console, event,
        QColor(), descriptor().display_name(),
        "test",
        {},
        feed.snapshot(), false
    );
#endif



#if 0
    DiscordWebHook::send_message(
        env.console, TABLE[0].ping,
        TABLE[0].tags,
        ":middle_finger:", QJsonArray(),
        ImageAttachment()
    );
#endif


//    connect_to_internet_with_inference(env, env.console);


#if 0
    std::vector<std::string> tokens;
    std::istringstream stream(" asdf qwer   xcvb  ");
    std::string str;
    while (std::getline(stream, str, ' ')){
        if (!str.empty()){
            cout << "asdf = " << str << endl;
            tokens.emplace_back(std::move(str));
        }
    }
#endif




//    all_bosses();




//    InferenceBoxScope box(overlay, 0.200, 0.100, 0.300, 0.065, Qt::blue);


//    dump_image(env.logger(), "test", "label", feed.snapshot());

#if 0
    std::string ERROR_REPORTING_URL = "https://discord.com/api/webhooks/892647720223924254/xcOc0SOPmiM7FVsRLZ4zKxqYEW5LGnfMwpfFdbaPH2ZhJnTnoOhDU8s_LiDGwj3SRm4p";
    std::string TELEMETRY_URL = "https://discord.com/api/webhooks/892648029352525914/o4Zqdsl2BN8bRUg1FXy63GmMjGZWvAFGxZPaxtl90yy67B-haEp2GL91XIpy1-5UykXz";

    flip(ERROR_REPORTING_URL);
    flip(TELEMETRY_URL);
//    cout << ERROR_REPORTING_URL << endl;
//    cout << TELEMETRY_URL << endl;

    for (size_t c = 0; c < TELEMETRY_URL.size(); c++){
        cout << (unsigned)(uint8_t)TELEMETRY_URL[c] << ",";
    }
    cout << endl;
#endif

#if 0
    QImage image0("ErrorDumps (old)/20210831-014455-PathPartyReader-ReadSprites.png");
    const uint32_t* pixels = (const uint32_t*)image0.bits();
    size_t bytes_per_row = image0.bytesPerLine();

    BinaryImage bin0(127, 20);
    filter_min_rgb32(bin0, pixels + 30*bytes_per_row/4, bytes_per_row, 255, 96, 128, 128);
    cout << "---------" << endl;
    cout << bin0.dump() << endl;


#if 1
    MinRgbFilter_Default filter(255, 96, 128, 128);
    BinaryImage_x64_SSE42 bin1(127, 20);
    rgb32_to_binary_image(bin1, pixels + 30*bytes_per_row/4, bytes_per_row, filter);
    cout << "---------" << endl;
    cout << bin1.dump() << endl;
#endif
#endif



//    YCommMenuDetector detector(false);
//    AsyncVisualInferenceSession session(env, feed, overlay);
//    session += detector;

#if 0
    QImage image0("ErrorDumps (old)/20210831-014455-PathPartyReader-ReadSprites.png");
    const uint32_t* pixels = (const uint32_t*)image0.bits();
    size_t bytes_per_row = image0.bytesPerLine();

    BinaryImage_Default bin0   = filter_min_rgb_Default  (127, 20, pixels + 30*bytes_per_row/4, bytes_per_row, 255, 96, 128, 128);
    BinaryImage_x64_SSE42 bin1 = filter_min_rgb_x64_SSE42(127, 20, pixels + 30*bytes_per_row/4, bytes_per_row, 255, 96, 128, 128);

    cout << bin0.dump() << endl;
    cout << "---------" << endl;
    cout << bin1.dump() << endl;

    MinRgbFilter_x64_SSE41 filter(255, 96, 128, 128);
    BinaryImage_x64_SSE42 bin2(127, 20);
    rgb32_to_binary_image(bin2, pixels + 30*bytes_per_row/4, bytes_per_row, filter);
    cout << "---------" << endl;
    cout << bin2.dump() << endl;
#endif


#if 0
    SelectionArrowFinder arrow_detector(env.console, ImageFloatBox(0.550, 0.600, 0.200, 0.200));
//    AsyncVisualInferenceSession session(env, feed, overlay);
//    session += arrow_detector;

    arrow_detector.detect(feed.snapshot());
#endif


#if 0
    __m128i pixel = _mm_set1_epi32(0xffffffff);
    __m128i scale0 = _mm_set1_epi16(256);
    __m128i scale1 = _mm_set1_epi16(256);

    __m128i pixel0 = _mm_and_si128(pixel, _mm_set1_epi16(0x00ff));
//    __m128i pixel1 = _mm_and_si128(_mm_srli_epi16(pixel, 8), _mm_set1_epi16(0x00ff));
    __m128i pixel1 = _mm_shuffle_epi8(pixel, _mm_setr_epi8(1, -1, 3, -1, 5, -1, 7, -1, 9, -1, 11, -1, 13, -1, 15, -1));

    pixel0 = _mm_mullo_epi16(pixel0, scale0);
    pixel1 = _mm_mullo_epi16(pixel1, scale1);

    pixel0 = _mm_shuffle_epi8(pixel0, _mm_setr_epi8(1, -1, 3, -1, 5, -1, 7, -1, 9, -1, 11, -1, 13, -1, 15, -1));
    pixel1 = _mm_and_si128(pixel1, _mm_set1_epi16(0xff00));

    pixel = _mm_or_si128(pixel0, pixel1);

    print8(pixel0); cout << endl;
    print8(pixel1); cout << endl;
#endif


#if 0
    QImage image0("ErrorDumps (old)/20210831-014455-PathPartyReader-ReadSprites.png");
    QImage image1("ErrorDumps (old)/20210831-020939-PathPartyReader-ReadSprites.png");
    image0 = image0.copy(0, 0, image0.width() - 1, image0.height());
    image1 = image1.copy(0, 0, image1.width() - 1, image1.height());

    auto time0 = std::chrono::system_clock::now();
    ImageStats stats = image_stats(image0);
//    double diff = ImageMatch::pixel_RMSD(image0, image1);

    auto time1 = std::chrono::system_clock::now();
    cout << std::chrono::duration_cast<std::chrono::nanoseconds>(time1 - time0).count() << endl;
    cout << stats.average << stats.stddev << endl;
//    cout << diff << endl;
#endif


#if 0
    uint64_t bits0 = filter_min_rgb_u64((const uint32_t*)image.bits()     , 63, 96, 0, 0);
    uint64_t bits1 = filter_min_rgb_u64((const uint32_t*)image.bits() + 64, 63, 96, 0, 0);
    cout << bits0 << endl;
    cout << bits1 << endl;

    __m128i mask = _mm_set1_epi32(0xff600000);
    mask = _mm_xor_si128(mask, _mm_set1_epi8(0x80));
    __m128i vbits = filter_min_rgb_SSE4((const uint32_t*)image.bits(), 63, mask);
    cout << vbits.m128i_u64[0] << endl;
    cout << vbits.m128i_u64[1] << endl;
#endif


//    uint8_t x[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

//    PartialWordLoader_SSE4 loader(16);
//    print8(loader.load(x)); cout << endl;

#if 0
    BinaryImage_SSE4 image(10, 4);
    image.set_zero();
    cout << image.pixel(1, 1) << endl;
    image.set_pixel(1, 1, true);
    cout << image.pixel(1, 1) << endl;
//    image.set_pixel(1, 1, false);
//    cout << image.pixel(1, 1) << endl;

    cout << image.dump() << endl;
#endif


//    __m128i x = _mm_setr_epi32(4, 0, 0, 0);
//    cout << least_significant_bit(x) << endl;
//    cout << most_significant_bit(x) << endl;

//    cout << Intrinsics_SSE4::low_bit(x) << endl;
//    cout << Intrinsics_SSE4::high_bit(x) << endl;

//    cout << most_significant_bit(7) << endl;
//    cout << most_significant_bit(15) << endl;


//    __m128i x = _mm_set1_epi64x(0xffffffffffffffff);


#if 0
    uint64_t x = 0xffffffffffffffff;

    uint64_t p, s;
//    popcount_sumindex(p, s, x);
    Intrinsics_u64::popcount_sumindex(p, s, x);

    cout << p << endl;
    cout << s << endl;
#endif

//    popcount;
//    sumcoord;

#if 0
    {
        AlignedVector<std::string> vector;

        for (const auto& item : vector){
            cout << item << endl;
        }

        vector.emplace_back("asdf");
        vector.emplace_back("qwer");
        for (const auto& item : vector){
            cout << item << endl;
        }

        vector.pop_back();
        for (const auto& item : vector){
            cout << item << endl;
        }
        vector.emplace_back("zxcv");
        vector.emplace_back("qwer");
        for (const auto& item : vector){
            cout << item << endl;
        }
    }
    cout << TrackingObject::live() << endl;
#endif


#if 0
    __m128i m = _mm_setr_epi32(-1, -1, -1, 1);
    __m128i x = _mm_setr_epi32(1, 0, 0, 0);

    cout << Intrinsics_SSE4::to_str(Intrinsics_SSE4::add(m, x)) << endl;

//    cout << Intrinsics_SSE4::expand(x, m) << endl;
//    cout << Intrinsics_SSE4::to_str(x) << endl;
//    cout << Intrinsics_SSE4::to_str(m) << endl;
#endif


#if 0
    __m128i x = _mm_set1_epi32(0x12345678);
    cout << Intrinsics_SSE4::to_str(x) << endl;
    x = bit_reverse(x);
    cout << Intrinsics_SSE4::to_str(x) << endl;
    x = bit_reverse(x);
    cout << Intrinsics_SSE4::to_str(x) << endl;
#endif

#if 0
    uint64_t matrix[2][2] = {
        {0x7777777777777777, 0x7777777777777777},
        {0xdddddddddddddddd, 0xdddddddddddddddd},
    };
    cout << Mask64(matrix[0][0]) << " " << Mask64(matrix[0][1]) << endl;
    cout << Mask64(matrix[1][0]) << " " << Mask64(matrix[1][1]) << endl;

    FillQueue<Intrinsics_u64> queue(2, 2);
    queue.push_point(1, 0, 1);



    while (true){
        FillQueue<Intrinsics_u64>::Point point;
        if (!queue.pop(point)){
            break;
        }

//        cout << "------------------> [" << point.x << "," << point.y << "]" << endl;

#if 0
        uint64_t mask = matrix[point.y][point.x];
//        cout << "mask = " << Mask64(mask) << endl;
        uint64_t x = point.mask & mask;
//        cout << "olap = " << Mask64(x) << endl;
        if (x == 0){
            continue;
        }
        x = fill_full(x, mask);
//        cout << "fill = " << Mask64(x) << endl;
        mask &= ~x;
//        cout << "mask = " << Mask64(mask) << endl;
        matrix[point.y][point.x] = mask;
#endif
        uint64_t x = point.mask;
        if (Intrinsics_u64::expand(x, matrix[point.y][point.x])){
            queue.push_neighbors(point.x, point.y, x);
        }

//        cout << Mask64(matrix[0][0]) << " " << Mask64(matrix[0][1]) << endl;
//        cout << Mask64(matrix[1][0]) << " " << Mask64(matrix[1][1]) << endl;
    }

    cout << Mask64(matrix[0][0]) << " " << Mask64(matrix[0][1]) << endl;
    cout << Mask64(matrix[1][0]) << " " << Mask64(matrix[1][1]) << endl;
#endif


#if 0
    uint64_t row0 = 0x7777777777777777;
    uint64_t row1 = 0xdddddddddddddddd;
    cout << Mask64(row0) << endl;
    cout << Mask64(row1) << endl;
    cout << "------------------" << endl;

    uint64_t x0 = 1;

    x0 = fill_full(x0, row0);
    cout << Mask64(x0) << endl;
    row0 &= ~x0;

    cout << "------------------" << endl;
    cout << Mask64(row0) << endl;
    cout << Mask64(row1) << endl;

    uint64_t x1 = x0 & row1;
    if (x1){}

    x1 = fill_full(x1, row1);
    cout << Mask64(x1) << endl;
    row1 &= ~x1;

    cout << "------------------" << endl;
    cout << Mask64(row0) << endl;
    cout << Mask64(row1) << endl;
#endif


#if 0
    Mask64 mask = 8480658636838181070;
    cout << mask << endl;

    Mask64 x = 0b100;

    x = fill_full(x, mask);
    cout << x << endl;
#endif


#if 0
    PokemonSelectMenuDetector detector(false);
//    AsyncVisualInferenceSession session(env, feed, overlay);
//    session += detector;

    QImage image("ErrorDumps (old)/20210831-172559-MaxLair-read_name_sprite.png");
    cout << detector.process_frame(image, std::chrono::system_clock::now()) << endl;
#endif


//    InferenceBoxScope box(overlay, 0.550, 0.600, 0.200, 0.200);

//    LobbyAllReadyDetector detector(4, std::chrono::system_clock::now());
//    AsyncVisualInferenceSession session(env, feed, overlay);
//    session += detector;

#if 0
    FrozenScreenDetector detector(std::chrono::milliseconds(1000), 10);
    AsyncVisualInferenceSession session(env, feed, overlay);
    session += detector;
#endif


#if 0
    QImage screen0("20210918-153851.jpg");
    QImage screen1("20210919-054635.jpg");

    cout << image_diff_total(screen0, screen1) << endl;
    cout << ImageMatch::pixel_RMSD(screen0, screen1) << endl;
#endif


#if 0
    PathReader reader(overlay, 0);
    std::string slugs[4];
//    double hp[4];
//    reader.read_hp(env.logger(), QImage("ErrorDumps/20210909-183539-PathPartyReader-ReadHP.png"), hp);
    QImage image("ErrorDumps (old)/20210831-025335-PathPartyReader-ReadSprites.png");
//    QImage image = feed.snapshot();
    reader.read_sprites(env.logger(), image, slugs);
#endif


#if 0
    QImage screen0("20210918-153851.jpg");
    QImage screen1("20210919-054635.jpg");

    auto time0 = std::chrono::system_clock::now();
    uint64_t count = 0;
    uint64_t sumsqrs = 0;
    sum_sqr_deviation(
        count, sumsqrs,
        screen0.width(), screen0.height(),
        (const uint32_t*)screen0.bits(), screen0.bytesPerLine(),
        (const uint32_t*)screen1.bits(), screen1.bytesPerLine(),
        0
    );
    auto time1 = std::chrono::system_clock::now();
    cout << std::chrono::duration_cast<std::chrono::nanoseconds>(time1 - time0).count() << endl;

    cout << std::sqrt((double)sumsqrs / (double)count) << endl;

    cout << ImageMatch::pixel_RMSD(screen0, screen1) << endl;
#endif


#if 0
    QImage screen("20210918-153851.jpg");

    auto time0 = std::chrono::system_clock::now();
    ImageStats stats0 = image_stats(screen);
    auto time1 = std::chrono::system_clock::now();
    cout << stats0.average << stats0.stddev << endl;
    cout << std::chrono::duration_cast<std::chrono::nanoseconds>(time1 - time0).count() << endl;


    QRgb average = ImageMatch::pixel_average(screen);
    cout << qRed(average) << "," << qGreen(average) << "," << qBlue(average) << endl;
#endif


#if 0
    auto time2 = std::chrono::system_clock::now();
    Kernels::PixelSums sums;
    Kernels::pixel_mean(
        sums, screen.width(), screen.height(),
        (const uint32_t*)screen.bits(), screen.bytesPerLine(),
        (const uint32_t*)screen.bits(), screen.bytesPerLine()
    );

    ImageStats stats1;
    FloatPixel sum(sums.sumR, sums.sumG, sums.sumB);
    FloatPixel sqr(sums.sqrR, sums.sqrG, sums.sqrB);

    stats1.average = sum / sums.count;
    FloatPixel variance = (sqr - sum*sum / sums.count) / (sums.count - 1);
    stats1.stddev = FloatPixel(
        std::sqrt(variance.r),
        std::sqrt(variance.g),
        std::sqrt(variance.b)
    );

    auto time3 = std::chrono::system_clock::now();
    cout << stats1.average << stats1.stddev << endl;
    cout << std::chrono::duration_cast<std::chrono::nanoseconds>(time3 - time2).count() << endl;
#endif


#if 0
//    Kernels::PixelSums sums;

    uint32_t pixel0[4] = {
        0x81123456,
        0x81123456,
        0x81123456,
        0x81123456,
    };
    uint32_t pixel1[4] = {
        0x81123456,
        0,
        0x81123456,
        0,
    };

    uint64_t count = 0;
    uint64_t sumsqr = 0;
    Kernels::sum_sqr_deviation(
        count, sumsqr, 4, 1, pixel, pixel
    );
#endif


//    qRed();


#if 0
    QImage screen("ErrorDumps/20210921-015058-ReadPath.png");
    test_find_symbols(env, overlay, {0.150, 0.020, 0.800, 0.780}, screen, 0.20);
#endif


#if 0
    ShinySparkleDetector detector(env.logger(), overlay, ImageFloatBox(0.5, 0.05, 0.5, 0.70));

    AsyncVisualInferenceSession session(env, feed, overlay);
    session += detector;
#endif


#if 0
    DenMonReader reader(env.logger(), overlay);
    reader.read(feed.snapshot());
#endif

//    pbf_move_right_joystick(env.console, 128, 0, 80, 2 * 125);
//    pbf_move_right_joystick(env.console, 128, 0, 100, 2 * 125);
//    pbf_move_right_joystick(env.console, 128, 255, 200, 2 * 125);
//    botbase.wait_for_all_requests();



//    for (const auto& item : all_type_sprites()){
//        item.second.matching_image().save(QString::fromStdString(item.second.slug() + ".png"));
//    }

#if 0
    PathMap map;
    map.path_type = 2;
    map.mon3[0] = PokemonType::BUG;
    map.mon3[1] = PokemonType::DARK;
    map.mon3[2] = PokemonType::DRAGON;
    map.mon3[3] = PokemonType::ELECTRIC;
    map.mon2[0] = PokemonType::FAIRY;
    map.mon2[1] = PokemonType::FIGHTING;
    map.mon2[2] = PokemonType::FIRE;
    map.mon2[3] = PokemonType::FLYING;
    map.mon1[0] = PokemonType::GHOST;
    map.mon1[1] = PokemonType::GRASS;

    auto paths = generate_paths(map, 0, -1);
    cout << "Paths = " << paths.size() << endl;
    for (const auto& path : paths){
        for (const auto& item : path){
            cout << "[" << (int)item.path_slot << ":" << get_type_slug(item.type) << "] ";
        }
        cout << endl;
    }
#endif



#if 0
//    QImage screen = feed.snapshot();
//    QImage screen("ErrorDumps (old)/20210831-025634-PathPartyReader-ReadSprites.png");
    QImage screen("ErrorDumps (old)/20210924-132645-ReadPathSide.png");
//    QImage screen("ErrorDumps (old)/20210919-203328-ReadPathSide.png");

//    InferenceBoxScope box(overlay, 0.150, 0.100, 0.800, 0.700);

//    QImage image = extract_box(screen, box);

//    cout << (int)find_side(image) << endl;

    PathReader reader(overlay, 0);
    GlobalState state;

    reader.read_side(env.logger(), state, screen);

#endif

#if 0
    CellMatrix matrix(image);
    BrightFilter filter(500);
    matrix.apply_filter(image, filter);

    std::vector<FillGeometry> objects = find_all_objects(matrix, 1, false, 300);

    std::deque<InferenceBoxScope> hits;
    for (const FillGeometry& item : objects){
        if (!is_arrow(image, matrix, item)){
            continue;
        }
        cout << item.area << " / " << item.box.area() << endl;
        hits.emplace_back(overlay, translate_to_parent(screen, box, item.box), Qt::green);
    }
#endif



#if 0
//    QImage screen("ErrorDumps (old)/20210831-014455-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-020939-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-025335-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-025634-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-040810-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-040811-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-043447-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-043448-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-045016-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-064752-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-113635-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-115647-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-163931-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-185522-PathPartyReader-ReadSprites.png");
//    QImage screen("ErrorDumps (old)/20210831-185523-PathPartyReader-ReadSprites.png");
    QImage screen("ErrorDumps/20210917-125152-ReadPath.png");
//    QImage screen = feed.snapshot();

    InferenceBoxScope box(overlay, 0.150, 0.100, 0.800, 0.700);

    QImage image = extract_box(screen, box);

    std::multimap<double, std::pair<PokemonType, ImagePixelBox>> candidates = find_symbols(image, 0.2);

    std::deque<InferenceBoxScope> hits;
    size_t c = 0;
    for (const auto& item : candidates){
        cout << get_type_slug(item.second.first) << ": " << item.first << endl;
        hits.emplace_back(overlay, translate_to_parent(screen, box, item.second.second), Qt::green);

        const ImagePixelBox& box = item.second.second;
        QImage img = image.copy(
            box.min_x, box.min_y,
            box.width(), box.height()
        );
//        img.save("test-" + QString::number(c++) + ".png");
    }

#endif




#if 0
    QImage screen("ErrorDumps (old)/20210831-014455-PathPartyReader-ReadSprites.png");
//    QImage screen = feed.snapshot();

    InferenceBoxScope box(overlay, 0.150, 0.200, 0.800, 0.600);

    QImage image = extract_box(screen, box);
//    image.save("test.png");


    CellMatrix matrix(image);
    BrightFilter filter(600);
    matrix.apply_filter(image, filter);


    std::vector<FillGeometry> objects = find_all_objects(matrix, 1, false, 20);

#if 0
    std::deque<InferenceBoxScope> hits;
    for (const FillGeometry& item : objects){
        cout << "[" << item.center_x() << "," << item.center_y() << "] - " << item.area << endl;
        hits.emplace_back(overlay, translate_to_parent(screen, box, item.box), Qt::green);
    }
#endif

    std::map<CellMatrix::ObjectID, FillGeometry> objmap;
    for (const FillGeometry& item : objects){
        objmap[item.id] = item;
    }

    for (auto& item0 : objmap){
        for (auto& item1 : objmap){
            if (item0.first >= item1.first){
                continue;
            }
            size_t distance = distance_sqr(item0.second.box, item1.second.box);
            if (distance < 5*5){
                item0.second.merge_assume_no_overlap(item1.second);
                objmap.erase(item1.second.id);
            }
        }
    }

    std::deque<InferenceBoxScope> hits;
    for (const auto& item : objmap){
//        cout << "[" << item.center_x() << "," << item.center_y() << "] - " << item.area << endl;

        QImage img = image.copy(
            item.second.box.min_x, item.second.box.min_y,
            item.second.box.width(), item.second.box.height()
        );
//        img.save("test-" + QString::number(item.first) + ".png");
        PokemonType type = match_type_symbol(img, item.first);
        if (type != PokemonType::NONE){
            hits.emplace_back(overlay, translate_to_parent(screen, box, item.second.box), Qt::green);
//            cout << get_type_slug(type) << " : " << item.second.box.area() << endl;
        }
    }

#endif




//    std::multimap<pxint_t, const FillGeometry*> horizontal;
//    std::multimap<pxint_t, const FillGeometry*> vertical;



#if 0
    QImage sprite(PERSISTENT_SETTINGS().resource_path + "PokemonSwSh/Types/ice.png");
    if (sprite.format() != QImage::Format_RGB32 && sprite.format() != QImage::Format_ARGB32){
        sprite = sprite.convertToFormat(QImage::Format_RGB32);
    }

    //  Set all non-255 alphas to zero.
    size_t words = sprite.bytesPerLine() / sizeof(uint32_t);
    uint32_t* ptr = (uint32_t*)sprite.bits();
    for (int r = 0; r < sprite.height(); r++){
        for (int c = 0; c < sprite.width(); c++){
//            cout << qAlpha(sprite.pixel(c, r)) << " ";
            uint32_t pixel = ptr[c];
            if ((pixel >> 24) != 0xff){
                ptr[c] = 0;
            }
        }
//        cout << endl;
        ptr += words;
    }

//    sprite.save("test.png");



    CellMatrix matrix(sprite);
    WhiteFilter filter;
    matrix.apply_filter(sprite, filter);
    std::vector<FillGeometry> objects = find_all_objects(matrix, 1, true);

    FillGeometry object;
    for (const FillGeometry& item : objects){
        cout << item.center_x() << "," << item.center_y() << endl;
        object.merge_assume_no_overlap(item);
    }
    cout << object.center_x() << "," << object.center_y() << endl;
    cout << "[" << object.box.min_x << "," << object.box.min_y << "][" << object.box.max_x << "," << object.box.max_y << "]" << endl;







#endif





#if 0
    QImage image = get_pokemon_sprite("cradily").sprite().scaled(10, 10);
    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            QRgb pixel = image.pixel(c, r);
            cout << qAlpha(pixel) << " ";
        }
        cout << endl;
    }
#endif



#if 0
    BattleMenuDetector detector;
    std::deque<InferenceBoxScope> boxes;
    detector.make_overlays(boxes, overlay);

    cout << detector.detect(feed.snapshot()) << endl;
#endif


#if 0
    PathSelectDetector detector;
    std::deque<InferenceBoxScope> boxes;
    detector.make_overlays(boxes, overlay);
#endif






#if 0
    ProfessorSwapDetector detector(overlay);
    std::deque<InferenceBoxScope> boxes;
    detector.make_overlays(boxes, overlay);

    cout << detector.detect(feed.snapshot()) << endl;
#endif


#if 0
    PokemonSwapMenuDetector detector(false);
    std::deque<InferenceBoxScope> boxes;
    detector.make_overlays(boxes, overlay);

    QImage screen("swap-jpn.jpg");
//    QImage screen(feed.snapshot());

    cout << detector.detect(screen) << endl;
#endif


#if 0
    BattleMenuReader reader(overlay, Language::English);

//    reader.read_opponent_hp(env.logger(), feed.snapshot());
//    reader.read_opponent_hp(env.logger(), QImage("ErrorDumps/20210908-193353-BattleMenuReader-read_opponent_hp.png"));

//    Health hp[4];
//    reader.read_hp(env.logger(), QImage("ErrorDumps/20210912-195759-MaxLair-read_name_sprite.png"), hp, 0);

//    reader.read_own_mon(env.logger(), QImage("ErrorDumps/20210913-101915-MaxLair-read_name_sprite.png"));

//    std::set<std::string> slugs{"stunfisk", "stunfisk-galar"};
//    reader.disambiguate_opponent(env.logger(), slugs, QImage("ErrorDumps/20210917-172444-DisambiguateBoss.png"));


#endif


//    relative_damage(get_maxlair_mon("nidoqueen"), get_maxlair_mon("toxtricity-amped"), 0, false);
//    relative_damage(get_maxlair_mon("nidoqueen"), get_maxlair_mon("toxtricity-amped"), 3, false);



//    make_maxlair_rental_json();


#if 0
    PokemonSelectMenuReader reader(env.logger(), overlay, Language::English);
    QImage image("ErrorDumps (old)/20210901-193318-MaxLair-read_name_sprite.png");
    reader.read_option(image, 0);
    reader.read_option(image, 1);
    reader.read_option(image, 2);
#endif
#if 0
    PokemonSwapMenuReader reader(env.logger(), overlay, Language::English);

    QImage image("ErrorDumps (old)/20210920-212120-MaxLair-read_name_sprite.png");
//    QImage image = feed.snapshot();

    std::string slugs[2];
    reader.read_options(image, slugs);
//    reader.read_option(image, 1);
#endif


#if 0
    BattleMenuReader reader(overlay, Language::English);

    QImage image("ErrorDumps/20210921-160110-MaxLair-read_name_sprite.png");
//    QImage image = feed.snapshot();

//    image.copy(QRect(-100, 100, 200, 100)).save("edge.png");

//    double hp[4];
//    reader.read_hp(env.logger(), image, hp, 0);

    reader.read_own_mon(env.logger(), image);
#endif




//    InferenceBoxScope box(overlay, 0.074, 0.094 + 0*0.16315, 0.051, 0.007, Qt::blue);



#if 0
    PathPartyReader reader(overlay, 0);

    double hp[4];
    reader.read_hp(env.logger(), feed.snapshot(), hp);
#endif


#if 0
    InferenceBoxScope box(overlay, 0.360, 0.120, 0.280, 0.005);

    InferenceBoxScope box0(overlay, 0.400, 0.120, 0.240, 0.005);
    QImage screen = feed.snapshot();
    ImageStats stats = image_stats(extract_box(screen, box0));
    cout << stats.average << stats.stddev << endl;
#endif


#if 0
    {
        InferenceBoxScope box0(overlay, 0.069, 0.914, 0.020, 0.008);
        QImage screen = feed.snapshot();
        ImageStats stats = image_stats(extract_box(screen, box0));
        cout << stats.average << stats.stddev << endl;
    }
    {
        InferenceBoxScope box0(overlay, 0.069 + 0.204 - 0.020, 0.914, 0.020, 0.008);
        QImage screen = feed.snapshot();
        ImageStats stats = image_stats(extract_box(screen, box0));
        cout << stats.average << stats.stddev << endl;
    }
#endif

#if 0
    ProfessorSwapDetector arrow_finder(overlay);

//    AsyncVisualInferenceSession session(env, feed, overlay);
//    session += arrow_finder;

    cout << arrow_finder.detect(QImage("ErrorDump/20210908-164426-PathPartyReader-ReadSprites.png")) << endl;
#endif



#if 0
    BattleMoveArrowFinder detector(overlay);
//    AsyncVisualInferenceSession inference(env, env.console);
//    inference += detector;

    QImage image("fail-arrow.png");

    cout << (int)detector.detect(image) << endl;
#endif


#if 0
    QImage image("arrow-good.png");
    int width = image.width();
    int height = image.height();
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
            QRgb pixel = image.pixel(c, r);
            bool black = qRed(pixel) < 48 && qGreen(pixel) < 48 && qBlue(pixel) < 48;
            if (black){
                image.setPixel(c, r, 0xff000000);
            }else{
                image.setPixel(c, r, 0x00000000);
            }
        }
    }
    image.save("arrow-cleaned.png");
#endif


#if 0
    InferenceBoxScope box(overlay, 0.640, 0.600, 0.055, 0.380, Qt::yellow);

    QImage screen = feed.snapshot();
    QImage image = extract_box(screen, box);

    CellMatrix matrix(image);

    BlackFilter filter;
    matrix.apply_filter(image, filter);

    image.save("test.png");

    std::vector<FillGeometry> objects = find_all_objects(matrix, 1, false, 200);

    std::deque<InferenceBoxScope> boxes;
    for (const FillGeometry& object : objects){
        bool arrow = is_arrow(image, matrix, object);
        cout << arrow << endl;
//        if (arrow){
//            image.copy(object.box.min_x, object.box.min_y, object.box.width(), object.box.height()).save("arrow.png");
//        }
//        cout << object.area << endl;
        boxes.emplace_back(overlay, translate_to_parent(screen, box, object.box));
    }
#endif




#if 0
    send_program_notification(
        env.logger(), false, QColor(0x000000ff),
        "Test Message",
        descriptor().display_name()
    );
#endif









#if 0
//    PokemonSelectMenuReader reader(env.logger(), overlay, LANGUAGE);
//    PokemonSwapMenuReader reader(env.logger(), overlay, LANGUAGE);
    BattleMenuReader reader(overlay, Language::English);

    QImage image("ErrorDumps/20210929-074054726330-MaxLair-read_name_sprite.png");

    reader.read_own_mon(env.logger(), image);

//    QImage image = feed.snapshot();
//    cout << reader.read_opponent_hp(env.logger(), image) << endl;
//    cout << reader.read_own_hp(env.logger(), image) << endl;
//    double hp[4];
//    reader.read_hp(image, hp);
//     int8_t pp[4];
//     reader.read_own_pp(env.logger(), image, pp);


//    std::string mon[3];
//    reader.read_options(image, mon);
#endif


#if 0
    PokemonSwapMenuReader reader(env.logger(), overlay, LANGUAGE);

    QImage image = feed.snapshot();

    std::string mon[2];
    int8_t pp[4];
//    reader.read_pp(QImage("ErrorDumps/20210823-195515-MaxLair-NameOCR.png"), pp);
    reader.read_options(image, mon);
    reader.read_pp(image, pp);
#endif




    env.wait_for(std::chrono::seconds(60));


}





QJsonObject convert_move(const QJsonObject& obj){
    QJsonObject ret;
    ret["move"] = obj["name_id"];
    ret["category"] = obj["category"];
    ret["type"] = obj["type_id"];
    ret["base_power"] = obj["base_power"];
    ret["accuracy"] = obj["accuracy"];
    ret["PP"] = obj["PP"];
    ret["spread"] = obj["is_spread"];
    ret["correction_factor"] = obj["correction_factor"];
    ret["effective_power"] = obj["power"];
    return ret;
}
void make_maxlair_rental_json(){
    QJsonDocument doc = read_json_file("boss_pokemon.json");
    QJsonObject obj = doc.object();


    QJsonObject new_obj;
    for (auto iter = obj.begin(); iter != obj.end(); ++iter){
        QJsonObject mon = iter.value().toObject();
        QJsonObject new_mon;

        new_mon["type"] = mon["type_ids"];
        new_mon["ability"] = mon["ability_name_id"];
        new_mon["base_stats"] = mon["base_stats"].toObject()["py/tuple"];
        {
            QJsonArray moves = mon["moves"].toArray();
            QJsonArray new_moves;
            for (const auto& item : moves){
                new_moves.append(convert_move(item.toObject()));
            }
            new_mon["moves"] = new_moves;
        }
        {
            QJsonArray moves = mon["max_moves"].toArray();
            QJsonArray new_moves;
            for (const auto& item : moves){
                new_moves.append(convert_move(item.toObject()));
            }
            new_mon["max_moves"] = new_moves;
        }

        new_obj[iter.key()] = new_mon;
    }

    write_json_file("file.json", QJsonDocument(new_obj));
}

void maxlair_make_mapping_json(){
    QJsonObject map;

    {
        QJsonDocument doc = read_json_file("rental_pokemon.json");
        QJsonObject obj = doc.object();
        for (auto iter = obj.begin(); iter != obj.end(); ++iter){
            std::string slug = iter.key().toStdString();
            QJsonObject mon = iter.value().toObject();

            QJsonObject item;

            QJsonArray ocr;
            {
                auto* data = get_pokemon_name_nothrow(slug);
                if (data != nullptr){
                    ocr.append(iter.key());
                }else{
                    cout << "Missing OCR: " << slug << endl;
                }
            }

            QJsonArray sprite;
            {
                auto* data = get_pokemon_sprite_nothrow(slug);
                if (data != nullptr){
                    sprite.append(iter.key());
                }else{
                    cout << "Missing Sprite: " << slug << endl;
                }
            }

            item["OCR"] = ocr;
            item["Sprite"] = sprite;
            map[iter.key()] = item;
        }
    }
    {
        QJsonDocument doc = read_json_file("boss_pokemon.json");
        QJsonObject obj = doc.object();
        for (auto iter = obj.begin(); iter != obj.end(); ++iter){
            std::string slug = iter.key().toStdString();
            QJsonObject mon = iter.value().toObject();

            QJsonObject item;

            QJsonArray ocr;
            {
                auto* data = get_pokemon_name_nothrow(slug);
                if (data != nullptr){
                    ocr.append(iter.key());
                }else{
                    cout << "Missing OCR: " << slug << endl;
                }
            }

            QJsonArray sprite;
            {
                auto* data = get_pokemon_sprite_nothrow(slug);
                if (data != nullptr){
                    sprite.append(iter.key());
                }else{
                    cout << "Missing Sprite: " << slug << endl;
                }
            }

            item["OCR"] = ocr;
            item["Sprite"] = sprite;
            map[iter.key()] = item;
        }
    }

    write_json_file("MaxLairSlugMap.json", QJsonDocument(map));
}



#if 0
class TrackingObject{
public:
    TrackingObject(std::string x){
        auto iter = m_live.find(this);
        if (iter != m_live.end()){
            PA_THROW_StringException("Double allocation.");
        }
        m_live.insert(this);
        m_data = x;
    }
    TrackingObject(TrackingObject&& x){
        auto iter = m_live.find(this);
        if (iter != m_live.end()){
            PA_THROW_StringException("Double allocation.");
        }
        m_live.insert(this);
        m_data = std::move(x.m_data);
    }
    TrackingObject(const TrackingObject&) = delete;
    void operator=(const TrackingObject&) = delete;
    ~TrackingObject(){
        auto iter = m_live.find(this);
        if (iter == m_live.end()){
            PA_THROW_StringException("Dangling free.");
        }
        m_live.erase(this);
    }

    static size_t live(){
        return m_live.size();
    }

private:
    std::string m_data;
    static std::set<TrackingObject*> m_live;
};
std::set<TrackingObject*> TrackingObject::m_live;
#endif




}
}




