/*  Test Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
//#include <QSystemTrayIcon>
#include "Common/Cpp/Exception.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Qt/QtJsonTools.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
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
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageMatch/FilterToAlpha.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/ImageTools/CommonFilters.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/OCR/RawOCR.h"
#include "CommonFramework/OCR/Filtering.h"
#include "CommonFramework/OCR/StringNormalization.h"
#include "CommonFramework/OCR/TextMatcher.h"
#include "CommonFramework/OCR/LargeDictionaryMatcher.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "CommonFramework/ImageMatch/CroppedImageMatcher.h"
#include "CommonFramework/Tools/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
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
#include "PokemonSwSh/Resources/PokemonSwSh_MaxLairDatabase.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
#include "PokemonSwSh/Resources/PokemonSwSh_TypeSprites.h"
#include "CommonFramework/Tools/DiscordWebHook.h"
#include "Pokemon/Pokemon_Notification.h"
#include "TestProgram.h"

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
//        m_iv_checker_reader.languages()
#if 1
        {
//            Language::None,
            Language::English,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
            Language::Japanese,
            Language::Korean,
            Language::ChineseSimplified,
            Language::ChineseTraditional,
        },
        false
#endif
    )
    , DROPDOWN(
        "<b>Test Dropdown:</b>",
        {
            "case 0",
            "case 1",
            "case 2",
            "case 3",
        }, 0
    )
    , STRING_SELECT(
        "<b>Test Select:</b>",
        {
            "Bulbasaur",
            "Charmander",
            "Charmeleon",
            "Charizard",
            "Squirtle",
            "Chikorita",
        }, "Charmander"
    )
    , BALL_SELECT("<b>Ball Select:</b>", "master-ball")
    , FILTER(true, false)
    , BATCH()
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(DROPDOWN);
    PA_ADD_OPTION(STRING_SELECT);
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(FILTER);
    PA_ADD_OPTION(BATCH);
}





class VertexMap{



private:
    std::map<pxint_t, const FillGeometry*> m_hor;
    std::map<pxint_t, const FillGeometry*> m_ver;
};









void TestProgram::program(SingleSwitchProgramEnvironment& env){
    using namespace OCR;
    using namespace Pokemon;
    using namespace PokemonSwSh::MaxLairInternal;

    BotBase& botbase = env.console;
    VideoFeed& feed = env.console;
    VideoOverlay& overlay = env.console;


//    DenMonReader reader(env.logger(), overlay);
//    reader.read(feed.snapshot());


    pbf_move_right_joystick(env.console, 128, 0, 80, 2 * 125);
    pbf_move_right_joystick(env.console, 128, 0, 100, 2 * 125);
//    pbf_move_right_joystick(env.console, 128, 255, 200, 2 * 125);
    botbase.wait_for_all_requests();



//    for (const auto& item : all_type_sprites()){
//        item.second.matching_image().save(QString::fromStdString(item.second.slug() + ".png"));
//    }


#if 0
//    QImage screen("ErrorDumps (old)/20210831-014455-PathPartyReader-ReadSprites.png");
    QImage screen("ErrorDumps (old)/20210831-020939-PathPartyReader-ReadSprites.png");
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
//    QImage screen = feed.snapshot();

    InferenceBoxScope box(overlay, 0.150, 0.200, 0.800, 0.600);

    QImage image = extract_box(screen, box);

    std::multimap<double, std::pair<PokemonType, ImagePixelBox>> candidates = find_symbols(image);

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
    PathPartyReader reader(overlay, 0);
    std::string slugs[4];
//    double hp[4];
//    reader.read_hp(env.logger(), QImage("ErrorDumps/20210909-183539-PathPartyReader-ReadHP.png"), hp);
    QImage image("ErrorDumps (old)/20210831-014455-PathPartyReader-ReadSprites.png");
//    QImage image = feed.snapshot();
    reader.read_sprites(env.logger(), image, slugs);
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

    cout << detector.detect(feed.snapshot()) << endl;
#endif


#if 0
    BattleMenuReader reader(overlay, Language::English);

//    reader.read_opponent_hp(env.logger(), feed.snapshot());
//    reader.read_opponent_hp(env.logger(), QImage("ErrorDumps/20210908-193353-BattleMenuReader-read_opponent_hp.png"));

//    Health hp[4];
//    reader.read_hp(env.logger(), QImage("ErrorDumps/20210912-195759-MaxLair-read_name_sprite.png"), hp, 0);

//    reader.read_own_mon(env.logger(), QImage("ErrorDumps/20210913-101915-MaxLair-read_name_sprite.png"));

    reader.disambiguate_opponent(env.logger(), feed.snapshot());


#endif


//    relative_damage(get_maxlair_mon("nidoqueen"), get_maxlair_mon("toxtricity-amped"), 0, false);
//    relative_damage(get_maxlair_mon("nidoqueen"), get_maxlair_mon("toxtricity-amped"), 3, false);



//    make_maxlair_rental_json();


#if 0
    PokemonSelectMenuReader reader(env.logger(), overlay, Language::English);
    QImage image("ErrorDumps/20210910-073238-MaxLair-read_name_sprite.png");
    reader.read_option(image, 2);
#endif
#if 0
    PokemonSwapMenuReader reader(env.logger(), overlay, Language::English);

    QImage image("ErrorDumps/20210910-181920-MaxLair-read_name_sprite.png");
//    QImage image = feed.snapshot();

    std::string slugs[2];
    reader.read_options(image, slugs);
//    reader.read_option(image, 1);
#endif


#if 0
    BattleMenuReader reader(overlay, Language::English);

//    QImage image("ErrorDumps/20210910-232803-MaxLair-read_name_sprite.png");
    QImage image = feed.snapshot();

    image.copy(QRect(-100, 100, 200, 100)).save("edge.png");

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

    QImage image("ErrorDumps/20210901-005327-MoveSlot.png");

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

//    BattleMenuReader reader(overlay);

//    cout << reader.can_dmax(feed.snapshot()) << endl;


//    GlobalStateTracker tracker(env, 2);

//    run_entrance(env, env.console, true, tracker);

//    return;


//    InferenceBoxScope box(overlay, 0.600, 0.900, 0.140, 0.050);
//    InferenceBoxScope box(overlay, 0.782, 0.850, 0.030, 0.050);

//    ImageStats stats = image_stats(extract_box(feed.snapshot(), box));
//    cout << stats.average << stats.stddev << endl;


//    back_out_to_caught_screen(env, env.console);

//    run_caught_screen(env, env.console, false, QImage());



//    cout << count_catches(overlay, feed.snapshot()) << endl;


//    InferenceBoxScope box(overlay, 0.541, 0.779, 0.105, 0.186);
//    QImage image = extract_box(feed.snapshot(), box);

//    cout << dmax_circle_ready(image) << endl;

#if 0
    image = image.scaled(200, 200);

    image.save("box.png");

    int center_x = image.width() / 2;
    int center_y = image.height() / 2;
    cout << image.width() << " x " << image.height() << endl;

    FloatPixel sum;
    size_t total = 0;

    for (int r = 0; r < image.height(); r++){
        for (int c = 0; c < image.width(); c++){
            int dy = r - center_y;
            int dx = c - center_x;
            if (dx*dx + dy*dy < 72*72){
                image.setPixel(c, r, 0x0000ff);
                continue;
            }
            if (dx*dx + dy*dy > 80*80){
                image.setPixel(c, r, 0x0000ff);
                continue;
            }
            if (dy < -60){
                image.setPixel(c, r, 0x0000ff);
                continue;
            }
            if (-18 < dy && dy < 40){
                image.setPixel(c, r, 0x0000ff);
                continue;
            }
            sum += FloatPixel(image.pixel(c, r));
            total++;
        }
    }
    image.save("circle.png");

    sum /= total;
    cout << sum << endl;
#endif







#if 0
//    EntranceDetector detector(overlay, feed.snapshot());
//    PathSelectDetector detector(overlay);
//    ItemSelectDetector detector(overlay, false);
//    BattleMenuDetector detector(overlay);
//    PokemonCaughtMenuDetector detector(overlay);
//    RaidCatchDetector detector(overlay);
//    PokemonSwapMenuDetector detector(overlay, false);
    PokemonCaughtMenuDetector detector(overlay);


//    QImage image("ErrorDumps/20210827-224729-MaxLair-NameOCR.png");
    QImage image = feed.snapshot();
    cout << detector.detect(image) << endl;

//    read_raid_mon(env.logger(), overlay, image, LANGUAGE);

//    AsyncVisualInferenceSession session(env, feed);
//    session += detector;





#endif


//    InferenceBoxScope box0(overlay, 0.760, 0.400 + 0*0.133, 0.050, 0.030, Qt::blue);
//    InferenceBoxScope box1(overlay, 0.760, 0.400 + 1*0.133, 0.050, 0.030, Qt::blue);
//    InferenceBoxScope box2(overlay, 0.760, 0.400 + 2*0.133, 0.050, 0.030, Qt::blue);
//    InferenceBoxScope box3(overlay, 0.760, 0.400 + 3*0.133, 0.050, 0.030, Qt::blue);







//    PokemonSelectMenuReader reader(env.logger(), overlay, LANGUAGE);
//    cout << (int)reader.who_is_selecting(feed.snapshot()) << endl;

#if 0
    InferenceBoxScope box(overlay, 0.3, 0.010, 0.4, 0.10, Qt::green);
    QImage image = extract_box(feed.snapshot(), box);

    TextImageFilter filter = make_OCR_filter(image);
    filter.apply(image);

    image.save("test.png");

    env.log(OCR::ocr_read(Language::English, image));
#endif

#if 0
    QImage sprite = get_pokemon_sprite("garbodor").sprite();
    sprite.save("sprite.png");


    QImage screen = feed.snapshot();

    InferenceBoxScope box0(overlay, 0.002, 0.347 + 0*0.163, 0.071 - 0.005, 0.102);
    InferenceBoxScope box1(overlay, 0.002, 0.347 + 1*0.163, 0.071 - 0.005, 0.102);
    InferenceBoxScope box2(overlay, 0.002, 0.347 + 2*0.163, 0.071 - 0.005, 0.102);
    InferenceBoxScope box3(overlay, 0.002, 0.347 + 3*0.163, 0.071 - 0.005, 0.102);
//    InferenceBoxScope box3(overlay, 0.002, 0.510 + 3*0.166, 0.071 - 0.005, 0.102);
    QImage image = extract_box(screen, box2);
//    image.scaled(sprite.size()).save("image.png");
//    image.save("image.png");


//    ImageStats stats = image_border_stats(image);
//    cout << stats.average << stats.stddev << endl;

//    read_pokemon_sprite_on_white(env.logger(), image);
    PathPartyReader reader(overlay);
    std::string slugs[4];
    reader.read_sprites(env.logger(), screen, slugs);
#endif



#if 0
//    PokemonSelectMenuReader reader(env.logger(), overlay, LANGUAGE);
//    PokemonSwapMenuReader reader(env.logger(), overlay, LANGUAGE);
    BattleMenuReader reader(overlay);

//    QImage image("ErrorDumps/20210826-040952-BattleMenuReader-read_opponent_hp.png");
    QImage image = feed.snapshot();
//    cout << reader.read_opponent_hp(env.logger(), image) << endl;
//    cout << reader.read_own_hp(env.logger(), image) << endl;
//    double hp[4];
//    reader.read_hp(image, hp);
     int8_t pp[4];
     reader.read_own_pp(env.logger(), image, pp);


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


#if 0
    pbf_press_button(env.console, BUTTON_B, 5, 5);

    env.update_stats();

    end_program_callback(env.console);
    end_program_loop(env.console);
#endif


//    StandardBattleMenuDetector detector(overlay);



//    PokemonSelectMenu detector(feed);
//    cout << detector.detect(feed.snapshot()) << endl;

#if 0
    BattleBallReader reader(env.logger(), env.console, Language::English);
    cout << reader.read_ball(QImage()) << endl;
#endif

//    dump_image(QImage("test.jpg"), "test");

//    get_pokeball_sprite("luxury-ball").sprite().save("sprite.png");


#if 0
    PokemonSelectMenuReader reader(env.logger(), overlay, Language::English);
    reader.load(feed.snapshot());

    cout << reader.index() << endl;
#endif
#if 0
    InferenceBoxScope box0(feed, 0.02, 0.02, 0.40, 0.04);
    InferenceBoxScope box1(feed, 0.09, 0.18, 0.30, 0.10);
    InferenceBoxScope box2(feed, 0.15, 0.72, 0.10, 0.10);
    InferenceBoxScope box3(feed, 0.35, 0.80, 0.10, 0.10);
    InferenceBoxScope box4(feed, 0.87, 0.17, 0.03, 0.20);
    InferenceBoxScope box5(feed, 0.87, 0.43, 0.03, 0.20);
    InferenceBoxScope box6(feed, 0.87, 0.69, 0.03, 0.20);

    QImage screen = feed.snapshot();

    ImageStats stats0 = pixel_stats(extract_box(screen, box0));
    cout << "box0.average = " << stats0.average << endl;
    cout << "box0.stddev  = " << stats0.stddev << endl;

    ImageStats stats1 = pixel_stats(extract_box(screen, box1));
    cout << "box1.average = " << stats1.average << endl;
    cout << "box1.stddev  = " << stats1.stddev << endl;

    ImageStats stats2 = pixel_stats(extract_box(screen, box2));
    cout << "box2.average = " << stats2.average << endl;
    cout << "box2.stddev  = " << stats2.stddev << endl;

    ImageStats stats3 = pixel_stats(extract_box(screen, box3));
    cout << "box3.average = " << stats3.average << endl;
    cout << "box3.stddev  = " << stats3.stddev << endl;

    ImageStats stats4 = pixel_stats(extract_box(screen, box4));
    cout << "box4.average = " << stats4.average << endl;
    cout << "box4.stddev  = " << stats4.stddev << endl;

    ImageStats stats5 = pixel_stats(extract_box(screen, box5));
    cout << "box5.average = " << stats5.average << endl;
    cout << "box5.stddev  = " << stats5.stddev << endl;

    ImageStats stats6 = pixel_stats(extract_box(screen, box6));
    cout << "box6.average = " << stats6.average << endl;
    cout << "box6.stddev  = " << stats6.stddev << endl;
#endif




//    test_send_message2(env.logger());



//    DiscordWebHook::send_message(env.logger(), true, "asdf", QJsonArray());
//    DiscordWebHook::send_file("20210813-153702.png", &env.logger());



//    ExperienceGainDetector detector(feed);
//    cout << detector.detect(feed.snapshot()) << endl;


//    basic_catcher(env, env.console, LANGUAGE, "beast-ball");


//    ReceivePokemonDetector detector(feed);


#if 0
    send_program_error_notification(
        &env.logger(),
        m_descriptor.display_name(),
        "test error"
    );
#endif


#if 0
    BattleBallReader reader(env.logger(), env.console, LANGUAGE);


    pbf_press_button(botbase, BUTTON_X, 10, 125);
    botbase.wait_for_all_requests();
    move_to_ball(reader, botbase, feed, "park-ball");
#endif



#if 0
    BattleBallReader reader(env.logger(), env.console, LANGUAGE);

    QImage frame = feed.snapshot();
    std::string slug = reader.read_ball(frame);
    reader.read_quantity(frame);
    env.log(slug);
#endif


#if 0
    InferenceBoxScope box(feed, 0.710, 0.624, 0.18, 0.060);

    QImage frame = feed.snapshot();
    frame = extract_box(frame, box);

    auto filter = make_OCR_filter(frame);
    filter.apply(frame);

//    binary_filter_black_text(frame);
    frame.save("test.png");
    QString str = OCR::ocr_read(LANGUAGE, frame);
    env.log(str);

//    BattleBallInventoryReader reader(env.logger(), env.console);

//    QImage frame = feed.snapshot();
//    reader.read_quantity(frame);
#endif



//    cout << normalize_newlines("asdf\r\nasdf\n") << endl;


//    const QImage& ralts = get_pokemon_sprites("ralts").sprite();
//    const QImage& zard = get_pokemon_sprites("charizard").sprite();
//    cout << pixel_euclidean_distance_stddev(ralts, zard) << endl;
//    cout << pixel_euclidean_distance_stddev(ralts, ralts) << endl;

#if 0
    InferenceBoxScope box0(feed, 0.649, 0.624, 0.0335, 0.060);
//    InferenceBoxScope box1(feed, 0.914, 0.63, 0.04, 0.05);

    QImage sprite = get_pokeball_sprite("poke").sprite();
//    sprite = sprite.scaled(sprite.size());
//    sprite = sprite.scaled(sprite.width() * 8, sprite.height() * 8);
    sprite.save("sprite.png");

    float sprite_average = ImageMatch::average_brightness(sprite);
    cout << "Sprite: " << sprite_average << endl;
#if 0
    QRgb sprite_average = ImageMatch::pixel_average(sprite);
    cout << "Sprite: " << endl;
    cout << "    A = " << qAlpha(sprite_average) << endl;
    cout << "    R = " << qRed(sprite_average) << endl;
    cout << "    G = " << qGreen(sprite_average) << endl;
    cout << "    B = " << qBlue(sprite_average) << endl;
#endif

    QImage image = extract_box(feed.snapshot(), box0);
    image = image.scaled(sprite.size());
//    ball = ball.scaled(sprite.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    image.save("image.png");

    float image_average = ImageMatch::average_brightness(image, sprite);
    cout << "Image: " << image_average << endl;
#if 0
    QRgb image_average = ImageMatch::pixel_average(ball);
    cout << "Sprite:" << endl;
    cout << "    A = " << qAlpha(image_average) << endl;
    cout << "    R = " << qRed(image_average) << endl;
    cout << "    G = " << qGreen(image_average) << endl;
    cout << "    B = " << qBlue(image_average) << endl;
#endif

    image = ImageMatch::scale_brightness(image, sprite_average / image_average);
    image.save("image2.png");
//    image_average = ImageMatch::average_brightness(image, sprite);
//    cout << "Image: " << image_average << endl;
#endif

//    BattleBallInventoryReader reader(env.logger(), env.console);
//    reader.read_ball(feed.snapshot());



//    feed.snapshot().save("clefable.png");


//    reader.read_ball(feed.snapshot());

#if 0
    ImageMatch::ExactImageMatcher matcher;
    for (const auto& item : all_pokemon_sprites()){
        matcher.add(item.first, item.second.silhouette());
    }

    InferenceBoxScope box(feed, 0.098, 0.23, 0.285, 0.41);
    QImage cropped = extract_box(feed.snapshot(), box);
    matcher.scale_to_dimensions(cropped);
    QImage filtered = ImageMatch::black_filter_to_alpha(cropped);
    filtered.save("image.png");
#endif

#if 0
    ImageMatch::MatchResult result = matcher.match(filtered, false);
    for (const auto& item : result.slugs){
        cout << item.first << " : " << item.second << endl;
    }
#endif

#if 0
    const QImage& sprite = get_pokemon_sprite("snorlax-gmax").silhouette();
    sprite.save("sprite.png");

    cout << ImageMatch::pixel_RMSD_masked(sprite, filtered) << endl;
#endif

//    image_diff_greyscale(sprite, scaled).save("diff.png");

#if 0
//    QImage image("Bad-Gyarados.png");
    QImage image;

    DenMonReader reader(&env.logger(), overlay);
    auto results = reader.read(image);

//    if (results.slugs.slugs.empty() || results.slugs.slugs.begin()->first > 50){
//        //  No detection. Keep going.
//        dump_image(image, "ReadDenMon");
//    }
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







}
}




