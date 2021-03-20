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




