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
#include "TestProgram.h"


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




double image_diff(const QImage& x, const QImage& y){
    if (x.isNull() || y.isNull()){
        return -1;
    }
    if (x.width() != y.width()){
        return -1;
    }
    if (x.height() != y.height()){
        return -1;
    }

    double sum = 0;

    int width = x.width();
    int height = x.height();
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
            sum += euclidean_distance(x.pixel(c, r), y.pixel(c, r));
        }
    }


    return sum;
}







void TestProgram::program(SingleSwitchProgramEnvironment& env) const{

    BotBase& botbase = env.console;
    VideoFeed& feed = env.console;

#if 0
    QImage image("F:/star.jpg");
//    QImage image("F:/test3.png");
    cout << pixel_average(image) << endl;

    std::vector<std::vector<MapState>> filter = build_light_filter(image);
    std::vector<FillGeometry> stars = find_stars(filter);
    for (const auto& star : stars){
        cout << "Star: [{" << star.center_x << ", " << star.center_y << "}, "
             << star.max_y - star.min_y << " x " << star.max_x - star.min_x << " = "
             << star.area
             << "]" << endl;
        if (is_star(filter, star, image)){
            cout << "Is star!" << endl;
        }
    }
    image.save("stars-test1.png");
#endif

#if 0
    QImage image("F:/star.jpg");
    FlagMatrix matrix(image);
    matrix.apply_filter<BrightYellowLightFilter<0x01>>(image);

    std::vector<FillGeometry> objects = find_all_objects(
        matrix,
        0, matrix.height(),
        0, matrix.width(),
        0x01, 0x02
    );

    for (const FillGeometry& object : objects){
        is_star(image, matrix, object, 0x01);

    }
    image.save("stars-test1.png");

#endif

#if 1
//    InferenceBoxScope box(env.console, 0.0, 0.1, 0.6, 0.8);
    InferenceBoxScope box(env.console, 0.5, 0.2, 0.5, 0.55);

    size_t c = 0;
    while (true){
        auto start = std::chrono::system_clock::now();
        env.check_stopping();

        QImage image = feed.snapshot();
        image = extract_box(image, box);

//        ShinyImageDetection detection;
//        detection.detect(image, &env.logger);

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




