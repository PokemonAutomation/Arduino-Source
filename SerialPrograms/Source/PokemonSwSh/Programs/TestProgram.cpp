/*  Test Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Clientside/PrettyPrint.h"
#include "ClientSource/Libraries/Logging.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/StatsDatabase.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "CommonFramework/Inference/FillGeometry.h"
#include "CommonFramework/Inference/AnomalyDetector.h"
#include "CommonFramework/Inference/ColorClustering.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CommonFramework/Inference/TimeWindowStatTracker.h"
#include "CommonFramework/Inference/VisualInferenceSession.h"
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyFilters.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SparkleTrigger.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SquareTrigger.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SquareDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyTrigger.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_FishingDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_MarkFinder.h"
#include "PokemonSwSh_StartGame.h"
#include "TestProgram.h"

#include <fstream>

//#include <Windows.h>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

TestProgram::TestProgram()
    : SingleSwitchProgram(
        FeedbackType::REQUIRED, PABotBaseLevel::PABOTBASE_12KB,
        "Test Program",
        "",
        "Test Program"
    )
{}



#if 0
class AsyncCircle : public AsyncCommandSet{
public:
    using AsyncCommandSet::AsyncCommandSet;
    virtual void task() override{
        while (true){
            pbf_move_left_joystick(m_context, 128, 255, 32, 0);
            pbf_move_left_joystick(m_context, 255, 255, 32, 0);
            pbf_move_left_joystick(m_context, 255, 128, 32, 0);
            pbf_move_left_joystick(m_context, 255, 0, 32, 0);
            pbf_move_left_joystick(m_context, 128, 0, 32, 0);
            pbf_move_left_joystick(m_context, 0, 0, 32, 0);
            pbf_move_left_joystick(m_context, 0, 128, 32, 0);
            pbf_move_left_joystick(m_context, 0, 255, 32, 0);
        }
    }
};
#endif











void TestProgram::program(SingleSwitchProgramEnvironment& env) const{
//    BotBase& botbase = env.console;
//    VideoFeed& feed = env.console;

    pbf_mash_button(env.console, BUTTON_B, 600);
    env.wait(std::chrono::milliseconds(2000));
    env.console.botbase().stop_all_commands();
    cout << "asdf" << endl;

//    env.wait(std::chrono::milliseconds(1000));

    pbf_mash_button(env.console, BUTTON_A, 600);
//    pbf_wait(env.console, 600);
    pbf_press_button(env.console, BUTTON_X, 10, 10);



#if 0
    {
        VisualInferenceSession session(
            env, feed
        );
        session += [](const QImage& screen){
            cout << "asdf" << endl;
            return false;
        };
        session.run();
    }
    cout << "stop" << endl;
#endif



#if 0
    AsyncDispatcher dispatcher;

    AsyncCircle circle(env.console.botbase(), dispatcher);

    env.wait(std::chrono::seconds(5));
//    Sleep(10000);
    circle.cancel();
#endif



#if 0
    StatSet set;
    set.open_from_file("test.txt");

//    cout << set.to_str() << endl;

    set.save_to_file("test2.txt");
#endif


#if 0
//    start_game_from_home_with_inference(env, env.logger, env.console, true, 0, 0, true);

//    std::pair<uint8_t, uint8_t> coord = get_direction(-3, -1.0);
//    cout << "direction = " << (int)coord.first << ", " << (int)coord.second << endl;

//    Trajectory trajectory = get_trajectory_float(.09, .09);
//    cout << trajectory.distance_in_ticks << " : " << (int)trajectory.joystick_x << "," << (int)trajectory.joystick_y << endl;

    ShinyHuntTracker tracker(true);
    tracker.parse_and_append_line("Encounters: 100 - Star Shinies: 2 - Square Shinies: 1");
//    cout << tracker.to_str() << endl;

//    StatLine line(tracker);
//    cout << line.to_str() << endl;

//    StatLine line1("2021-04-08 00:25:12.131850 - Encounters: 100 - Star Shinies: 2 - Square Shinies: 1");
//    cout << line1.to_str() << endl;

    StatList list;
    list += tracker;
    list += "2021-04-08 00:25:12.131850 - Encounters: 200 - Star Shinies: 1 - Square Shinies: 0";

    cout << list.to_str() << endl;

    StatSet set;
    StatList& program0 = set["program 0"];
    program0 += "Timestamp - Encounters: 100 - Star Shinies: 2 - Square Shinies: 1";
    program0 += "Timestamp - Encounters: 200 - Star Shinies: 1 - Square Shinies: 0";
    StatList& program1 = set["program 1"];
    program1 += "Timestamp - Encounters: 300 - Star Shinies: 3 - Square Shinies: 2";
    program1 += "Timestamp - Encounters: 400 - Star Shinies: 4 - Square Shinies: 1";

    set.save_to_file("test.txt");
#endif


#if 0
    QImage image("test-screen.png");
//    QImage image("test-1617471750423682600-O.png");
//    QImage image = feed.snapshot();
//    image.save("square-test0.png");
    FillMatrix matrix(image);

    BrightYellowLightFilterDebug filter;
    matrix.apply_filter(image, filter);
    image.save("square-test0.png");

    std::vector<FillGeometry> objects;
    objects = find_all_objects(matrix, 1, true);
    cout << "objects = " << objects.size() << endl;

    std::deque<InferenceBoxScope> boxes;
    for (const FillGeometry& object : objects){
        if (is_square2(image, matrix, object)){
            InferenceBox box = translate_to_parent(image, InferenceBox(0, 0, 1, 1), object.box);
            box.color = Qt::green;
            boxes.emplace_back(feed, box);
        }
    }

    env.wait(std::chrono::seconds(600));
#endif


#if 0
    std::deque<InferenceBoxScope> grid;
    for (size_t r = 0; r < 10; r++){
        for (size_t c = 0; c < 10; c++){
            grid.emplace_back(feed, 0.1 * c, 0.1 * r, 0.1, 0.1);
        }
    }

    env.wait(std::chrono::seconds(5));

    pbf_move_left_joystick(208, 16, 500, 0);

    env.wait(std::chrono::seconds(600));
#endif



#if 0
    QImage screen("mark-test0.png");

    std::vector<PixelBox> marks;
    find_marks(screen, nullptr, &marks);

    for (const PixelBox& box : marks){
        cout << box.width() << " x " << box.height() << endl;
    }
#endif


#if 0
    FillMatrix blue_matrix(screen);
    BlueFilter blue_filter;
    blue_matrix.apply_filter(blue, blue_filter);
    blue.save("blue.png");
#endif



#if 0
    SummaryShinySymbolDetector detector(feed, env.logger());

    detector.wait_for_detection(env);

    env.wait(std::chrono::seconds(600));
#endif

#if 0
    detect_shiny_battle(
        env, env.console,
        SHINY_BATTLE_REGULAR,
        std::chrono::seconds(60)
    );
#endif


#if 0
    StandardBattleMenuDetector detector(feed);
    cout << "Battle Menu = " << detector.detect(feed.snapshot()) << endl;
    env.wait(std::chrono::seconds(600));
#endif

#if 0
    FishingDetector detector(feed);
    detector.wait_for_detection(env, env.logger);
#endif


#if 0
    QImage screen("FishingBig.jpg");

    FillMatrix matrix(screen);
    PinkFilter2 filter;
    matrix.apply_filter(screen, filter);
    screen.save("test.png");

    std::vector<FillGeometry> objects = find_all_objects(matrix, false);
    std::multimap<size_t, FillGeometry> candidate_top;
    std::multimap<size_t, FillGeometry> candidate_bot;
    for (const FillGeometry& object : objects){
        ImageStats stats = object_stats(screen, matrix, object);

        double aspect_ratio = (double)object.box.width() / object.box.height();
        FloatPixel color_ratio = stats.average / stats.average.sum();
        double stddev = stats.stddev.sum();

#if 0
        cout << object.area << " : [" << object.center_x << "," << object.center_y
             << "][" << object.box.width() << " x " << object.box.height()
             << "], mean = " << stats.average / stats.average.sum()
             << ", stddev = " << stats.stddev << endl;
#endif

        if (0.4 < aspect_ratio && aspect_ratio < 0.6 &&
            stddev < 50 &&
            euclidean_distance(color_ratio, FloatPixel(0.56, 0.14, 0.30)) < 0.2
        ){
//            cout << "top" << endl;
            candidate_top.emplace(object.area, object);
        }
        if (1.0 < aspect_ratio && aspect_ratio < 1.5 &&
            stddev < 25 &&
            euclidean_distance(color_ratio, FloatPixel(0.56, 0.21, 0.23)) < 0.2
        ){
//            cout << "bottom" << endl;
            candidate_bot.emplace(object.area, object);
        }
    }

    for (auto iter = candidate_top.rbegin(); iter != candidate_top.rend(); ++iter){
        const FillGeometry& top = iter->second;
        size_t top_area = top.area;
        size_t area_low = top_area / 7.;
        size_t area_high = top_area / 5.;
//        cout << "area = " << top_area << ", low = " << area_low << ", high = " << area_high << endl;
        auto iter0 = candidate_bot.lower_bound(area_low);
        auto iter1 = candidate_bot.upper_bound(area_high);
        for (; iter0 != iter1; ++iter0){
            const FillGeometry& bottom = iter0->second;

//            cout << "top_area = " << top_area << ", bot_area = " << bottom.area << endl;

            //  Verify that top is above bottom.
            if (top.box.max_y >= bottom.box.min_y){
                continue;
            }
//            cout << "check 1" << endl;

            //  Verify bottom is left of the top.
            if (top.center_x <= bottom.center_x){
                continue;
            }

            //  Make sure horizontal alignment is reasonable.
            int mid = (top.box.min_x + top.box.max_x) / 2;
            if (std::abs(bottom.box.max_x - mid) * 5 > top.box.width()){
                continue;
            }

            //  Make sure vertical alignment is reasonable.
            if (top.box.max_y + top.box.height() <= bottom.box.min_y){
                continue;
            }

            cout << "match!" << endl;
        }
    }
#endif




#if 0
    RaidCatchDetector detector(feed, std::chrono::seconds(60));

    size_t c = 0;
    while (true){
        auto start = std::chrono::system_clock::now();
        env.check_stopping();

        if (detector.has_timed_out()){
            break;
        }
        if (detector.detect()){
            break;
        }


        auto end = std::chrono::system_clock::now();
        auto duration = end - start;
//        cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << endl;
        if (duration < std::chrono::milliseconds(50)){
            env.wait(std::chrono::milliseconds(50) - duration);
        }
        c++;
//        break;
    }
#endif


//    QImage image = feed.snapshot();
//    cout << detector.detect(image) << endl;
//    cout << "box1 = " << cluster_distance_2(extract_box(image, box1), qRgb(255, 255, 255), qRgb(90, 180, 90)) << endl;
//    cout << "box1 = " << cluster_distance_2(extract_box(image, box1), qRgb(0, 0, 0), qRgb(90, 180, 90)) << endl;
//    cout << "box2 = " << cluster_distance_2(extract_box(image, box2), qRgb(255, 255, 255), qRgb(150, 132, 80)) << endl;
//    cout << "box3 = " << cluster_distance_2(extract_box(image, box3), qRgb(255, 255, 255), qRgb(140, 90, 180)) << endl;
//    cout << "box2 = " << cluster_distance_2(extract_box(image, box2), qRgb(0, 0, 0), qRgb(150, 132, 80)) << endl;
//    cout << "box3 = " << cluster_distance_2(extract_box(image, box3), qRgb(0, 0, 0), qRgb(140, 90, 180)) << endl;

#if 0
    QImage image("detection-381-O.png");

    ShinyImageDetection signatures;
    signatures.accumulate(image);
#endif

//    QImage image("battle-menu.png");
//    StandardBattleMenuDetector detector(env.console);
//    detector.detect(image);

#if 0
    ShinyEncounterDetector detector(
        env.console, env.logger,
        ShinyEncounterDetector::RAID_BATTLE,
        std::chrono::seconds(30)
    );
    detector.detect(env);
#endif

#if 0
//    InferenceBoxScope box(env.console, 0.0, 0.1, 0.6, 0.8);
//    InferenceBoxScope box(env.console, 0.5, 0.2, 0.5, 0.55);
//    InferenceBoxScope box(env.console, 0.3, 0.0, 0.4, 0.8);
//    StandardBattleMenuDetector battle_menu(env.console);
//    ShinyImageDetection shiny_animation;

//    QImage last;
//    std::deque<double> window;

//    TimeNormalizedDeltaAnomalyDetector detector(40, 255);


    size_t c = 0;
    while (true){
        auto start = std::chrono::system_clock::now();
        env.check_stopping();

        QImage image = feed.snapshot();
//        cout << battle_menu.detect(image) << endl;

        detector.detect();

//        cout << image.width() << " x " << image.height() << " : " << image.sizeInBytes() << endl;
//        image = extract_box(image, box);

//        double diff = image_diff(last, image);
//        last = std::move(image);
//        double sigma = detector.push(diff);
//        if (std::abs(sigma) > 3){
//            cout << "sigma = " << sigma << endl;
//            env.logger.log("Screen Anomaly: sigma = " + QString::number(sigma), "purple");
//        }

//        ShinyImageDetection shiny_signatures;
//        shiny_signatures.detect(image, &env.logger);

        auto end = std::chrono::system_clock::now();
        auto duration = end - start;
//        cout << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << endl;
        if (duration < std::chrono::milliseconds(50)){
            env.wait(std::chrono::milliseconds(50) - duration);
        }
        c++;
//        break;
    }
#endif



//    BeamReader reader(feed, env.logger);
//    if (!reader.run(env, botbase, 3 * TICKS_PER_SECOND)){
//        pbf_press_button(botbase, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
//    }

}







}
}
}




