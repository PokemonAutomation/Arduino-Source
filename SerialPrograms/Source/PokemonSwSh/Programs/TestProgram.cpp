/*  Test Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/FillGeometry.h"
#include "CommonFramework/Inference/AnomalyDetector.h"
#include "CommonFramework/Inference/ColorClustering.h"
#include "CommonFramework/Inference/FloatStatAccumulator.h"
#include "CommonFramework/Inference/TimeWindowStatTracker.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyFilters.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SparkleTrigger.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_SquareTrigger.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyTrigger.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SummaryShinySymbolDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_FishingDetector.h"
#include "PokemonSwSh_StartGame.h"
#include "TestProgram.h"

#include <fstream>

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




void TestProgram::program(SingleSwitchProgramEnvironment& env) const{
    BotBase& botbase = env.console;
    VideoFeed& feed = env.console;

//    start_game_from_home_with_inference(env, env.logger, env.console, true, 0, 0, true);

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
    SummaryShinySymbolDetector detector(feed, env.logger);

    detector.wait_for_detection(env);

    env.wait(std::chrono::seconds(600));
#endif

#if 1
    ShinyEncounterDetector detector(
        feed, env.logger,
        ShinyEncounterDetector::REGULAR_BATTLE,
        std::chrono::seconds(60)
    );
    detector.detect(env);
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




