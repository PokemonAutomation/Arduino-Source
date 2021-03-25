/*  Fishing Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/FillMatrix.h"
#include "PokemonSwSh_FishingDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

struct PinkFilter{
    size_t count = 0;

    void operator()(FillMatrix::ObjectID& cell, const QImage& image, size_t x, size_t y){
        QRgb pixel = image.pixel(x, y);
        int red = qRed(pixel);
        int green = qGreen(pixel);
//        int blue = qBlue(pixel);
        int set = red > 128 && green < 128;
        cell = set;
        count += set;
    }
};
struct PinkFilter2{
    size_t count = 0;

    void operator()(FillMatrix::ObjectID& cell, QImage& image, size_t x, size_t y){
        QRgb pixel = image.pixel(x, y);
        int red = qRed(pixel);
        int green = qGreen(pixel);
//        int blue = qBlue(pixel);
        int set = red > 128 && green < 128;
        if (!set){
            image.setPixel(x, y, 0);
        }
        cell = set;
        count += set;
    }
};


FishingDetector::FishingDetector(
    VideoFeed& feed
)
    : m_feed(feed)
    , m_hook_box(feed, 0.4, 0.15, 0.2, 0.4)
    , m_miss_box(feed, 0.3, 0.9, 0.4, 0.05)
    , m_battle_menu(feed)
{}
FishingDetector::Detection FishingDetector::detect_now(){
    QImage screen = m_feed.snapshot();

    if (m_battle_menu.detect(screen)){
        return Detection::BATTLE_MENU;
    }

    QImage hook_image = extract_box(screen, m_hook_box);
    {
        QImage image = extract_box(screen, m_miss_box);
        ImageStats stats = pixel_stats(image);
        if (stats.stddev.sum() < 10 && stats.average.sum() > 500 && pixel_stddev(hook_image).sum() > 50){
            return Detection::MISSED;
        }
    }

//    return Detection::NO_DETECTION;


    FillMatrix matrix(hook_image);
    PinkFilter filter;
    matrix.apply_filter(hook_image, filter);

    std::vector<FillGeometry> objects = find_all_objects(matrix, false, 10);
    std::multimap<size_t, FillGeometry> candidate_top;
    std::multimap<size_t, FillGeometry> candidate_bot;
    for (const FillGeometry& object : objects){
        ImageStats stats = object_stats(hook_image, matrix, object);
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
            stddev < 100 &&
            euclidean_distance(color_ratio, FloatPixel(0.56, 0.14, 0.30)) < 0.2
        ){
//            cout << "top" << endl;
            candidate_top.emplace(object.area, object);
        }
        if (1.0 < aspect_ratio && aspect_ratio < 1.5 &&
            stddev < 100 &&
            euclidean_distance(color_ratio, FloatPixel(0.56, 0.21, 0.23)) < 0.2
        ){
//            cout << "bottom" << endl;
            candidate_bot.emplace(object.area, object);
        }
    }
//    if (!candidate_top.empty()){
//        hook_image.save("test.png");
//    }

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
            if (std::abs(bottom.box.max_x - mid) * 3 > top.box.width()){
                continue;
            }

            //  Make sure vertical alignment is reasonable.
            if (top.box.max_y + top.box.height() <= bottom.box.min_y){
                continue;
            }

//            cout << "match!" << endl;
            return Detection::HOOKED;
        }
    }

    return Detection::NO_DETECTION;
}
FishingDetector::Detection FishingDetector::wait_for_detection(
    ProgramEnvironment& env, Logger& logger,
    std::chrono::seconds timeout
){
    auto start = std::chrono::system_clock::now();
    auto last = start;

    while (true){
        env.check_stopping();

        Detection detection = detect_now();
        switch (detection){
        case Detection::NO_DETECTION:
            break;
        case Detection::HOOKED:
            logger.log("FishEncounterDetector: Detected hook!", "purple");
            return detection;
        case Detection::MISSED:
            logger.log("FishEncounterDetector: Missed a hook.", "red");
            return detection;
        case Detection::BATTLE_MENU:
            logger.log("FishEncounterDetector: Expected battle menu.", "red");
            return detection;
        }

        auto now = std::chrono::system_clock::now();
        if (now - start > timeout){
            logger.log("FishEncounterDetector: Timed out.", "red");
            return Detection::NO_DETECTION;
        }
        auto time_since_last_frame = now - last;
        if (time_since_last_frame > std::chrono::milliseconds(50)){
            env.wait(std::chrono::milliseconds(50) - time_since_last_frame);
        }
        last = now;
    }
}



}
}
}

