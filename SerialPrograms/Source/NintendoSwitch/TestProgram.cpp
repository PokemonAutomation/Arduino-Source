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
#include "CommonFramework/ImageMatch/FilterToAlpha.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
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
#include "PokemonSwSh/Inference/PokemonSwSh_FishingDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "PokemonSwSh/Inference/PokemonSwSh_ReceivePokemonDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_PokemonSpriteReader.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleBallReader.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_ExperienceGainDetector.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PathSelect.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_ItemSelectMenu.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_EndBattle.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.h"
#include "PokemonSwSh/Programs/PokemonSwSh_BasicCatcher.h"
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




#if 0
ImageMatch::ExactImageMatcher make_POKEMON_SPRITE_MATCHER(){
    ImageMatch::ExactImageMatcher matcher;
    for (const auto& item : all_pokemon_sprites()){
        matcher.add(item.first, item.second.sprite());
    }
    return matcher;
}
const ImageMatch::ExactImageMatcher& POKEMON_SPRITE_MATCHER(){
    static ImageMatch::ExactImageMatcher matcher = make_POKEMON_SPRITE_MATCHER();
    return matcher;
}



class PokemonSpriteOnWhiteMatcher : public ImageMatch::CroppedImageMatcher{
public:
    virtual QRect get_enclosing_box(const QImage& image) const override{
        return ImageMatch::enclosing_rectangle_with_pixel_filter(
            image,
            [](QRgb pixel){
                return qRed(pixel) + qGreen(pixel) + qBlue(pixel) < 50;
            }
        );
    }
};
#endif




void TestProgram::program(SingleSwitchProgramEnvironment& env){
    using namespace OCR;
    using namespace Pokemon;
    using namespace PokemonSwSh::MaxLairInternal;

    BotBase& botbase = env.console;
    VideoFeed& feed = env.console;
    VideoOverlay& overlay = env.console;


//    PathSelectDetector detector(overlay);
//    ItemSelectDetector detector(overlay, false);
//    BattleMenuDetector detector(overlay);
//    PokemonCaughtMenuDetector detector(overlay);
//    RaidCatchDetector detector(overlay);
//    PokemonSwapMenuDetector detector(overlay, false);

//    cout << detector.detect(feed.snapshot()) << endl;

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

#if 1
//    PokemonSelectMenuReader reader(env.logger(), overlay, LANGUAGE);
//    PokemonSwapMenuReader reader(env.logger(), overlay, LANGUAGE);
    BattleMenuReader reader(overlay);

//    QImage image("ErrorDumps/20210826-040952-BattleMenuReader-read_opponent_hp.png");
    QImage image = feed.snapshot();
    cout << reader.read_opponent_hp(env.logger(), image) << endl;


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



//    DiscordWebHook::send_message(true, "asdf", QJsonArray(), &env.logger());
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




    env.wait(std::chrono::seconds(60));


}







}
}




