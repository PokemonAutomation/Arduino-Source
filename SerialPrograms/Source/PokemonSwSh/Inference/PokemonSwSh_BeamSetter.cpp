/*  Beam Setter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Drop a wishing piece and determine if it is red or purple.
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/PrettyPrint.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CommonFramework/Inference/TimeWindowStatTracker.h"
#include "PokemonSwSh_BeamSetter.h"

//#include "CommonFramework/Inference/FillGeometry.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


BeamSetter::BeamSetter(VideoFeed& feed, Logger& logger)
    : m_feed(feed)
    , m_logger(logger)
    , m_text_box(feed, Qt::red, 0.400, 0.825, 0.05, 0.05)
    , m_box(feed, Qt::red, 0.10, 0.005, 0.8, 0.470)
{
    for (size_t c = 0; c < 32; c++){
        m_boxes.emplace_back(0.10 + 0.025*c, 0.005, 0.025, 0.470);
    }
}



struct PurpleDetectionEntry{
#if 0
    double stddev;
    double brightness;
    double euclidean_delta;
#endif
};
struct PurpleDetectionAccumulator{
    using StatObject = PurpleDetectionEntry;

#if 0
    PurpleDetectionEntry oldest;
    PurpleDetectionEntry newest;

    double delta_stddev       () const{ return newest.stddev - oldest.stddev; }
    double delta_brightness   () const{ return newest.brightness - oldest.brightness; }
    double delta_euclidean    () const{ return newest.euclidean_delta - oldest.euclidean_delta; }

    double brightness_ratio   () const{ return (768 - newest.brightness) / (768 - oldest.brightness); }
#endif
};

//struct ImageDelta{
//    double average_euclidean;
//};
//struct ImageDeltaAccumulator{
//
//};



#if 0
struct PurpleBeamFilter{
    size_t count = 0;

    void operator()(FillMatrix::ObjectID& cell, const QImage& image, int x, int y){
        QRgb pixel = image.pixel(x, y);
        int set = (pixel & 0x00c0c0c0) == 0x00c0c0c0 ? 1 : 0;
        cell = set;
        count += set;
    }
};

struct PurpleBeamFilterDebug{
    size_t count = 0;

    void operator()(FillMatrix::ObjectID& cell, QImage& image, int x, int y){
        QRgb pixel = image.pixel(x, y);
        int set = (pixel & 0x00c0c0c0) == 0x00c0c0c0 ? 1 : 0;
        cell = set;
        if (cell == 0){
            image.setPixel(x, y, 0);
        }
        count += set;
    }
};
#endif



BeamSetter::Detection BeamSetter::run(
    ProgramEnvironment& env, BotBase& botbase,
    bool save_screenshot,
    uint16_t timeout_ticks,
    double min_brightness,
    double min_euclidean,
    double min_delta_ratio,
    double min_sigma_ratio
){
    //  Grab baseline image.
    QImage baseline_image = m_feed.snapshot();
    if (baseline_image.isNull()){
        m_logger.log("BeamSetter(): Screenshot failed.", "purple");
        return Detection::NO_DETECTION;
    }
//    baseline_image.save("f:/test0.jpg");
//    cout << "=======================" << endl;

    std::vector<FloatPixel> baseline_values(m_boxes.size());
    std::vector<FloatPixel> baseline_ratios(m_boxes.size());
    for (size_t c = 0; c < m_boxes.size(); c++){
        baseline_values[c] = pixel_average(extract_box(baseline_image, m_boxes[c]));
        baseline_ratios[c] = baseline_values[c] / baseline_values[c].sum();
    }

    //  Drop the wishing piece.
    pbf_press_button(botbase, BUTTON_A, 10, 10);
    botbase.wait_for_all_requests();

    //  Set up detection history.
//    std::map<size_t, size_t> red_detections;
//    std::map<size_t, size_t> purple_detections;

    bool low_stddev_flag = false;
//    std::vector<FloatPixel> current_values(m_boxes.size());
//    std::vector<FloatPixel> current_ratio_diffs(m_boxes.size());

//    static size_t c = 0;

//    std::vector<TimeWindowStatTracker<PurpleDetectionAccumulator>> trackers;
    std::vector<TimeWindowStatTracker<FloatStatAccumulator>> trackers;
    for (size_t c = 0; c < m_boxes.size(); c++){
        trackers.emplace_back(std::chrono::milliseconds(1000));
    }

//    std::vector<size_t> large_sigmas(m_boxes.size());

    InferenceThrottler throttler(
        std::chrono::milliseconds((uint64_t)timeout_ticks * 1000 / TICKS_PER_SECOND),
        std::chrono::milliseconds(50)
    );

//    static size_t count = 0;

    QImage last_screenshot = baseline_image;
    do{
        //  Take screenshot.
        QImage current_screenshot = m_feed.snapshot();
        if (current_screenshot.isNull()){
            m_logger.log("BeamSetter(): Screenshot failed.", "purple");
            return Detection::NO_DETECTION;
        }
//        current.save("f:/test1.jpg");

        //  Text detection.
        double text_stddev = pixel_stddev(extract_box(current_screenshot, m_text_box)).sum();
        if (text_stddev < 10){
            low_stddev_flag = true;
        }

//        FillMatrix matrix(current_screenshot);
//        PurpleBeamFilterDebug filter;
//        matrix.apply_filter(current_screenshot, filter);

//        current_screenshot.save("test-" + QString::number(count++) + ".png");


#if 1
        QImage baseline_diff = image_diff_greyscale(baseline_image, current_screenshot);
//        baseline_diff.save("diff-" + QString::number(c++) + ".png");
        auto now = std::chrono::system_clock::now();

        bool purple = false;
        size_t best_index = 0;
        double best_euclidean = 0;
        double best_stddev = 0;
        double best_brightness = 0;
//        PurpleDetectionAccumulator max_diff_delta;
//        FloatStatAccumulator max_diff_delta;
        double best_delta = 0;
        double best_sigma = 0;
        for (size_t c = 0; c < m_boxes.size(); c++){
            FloatStatAccumulator stats = trackers[c].accumulate_all();

            QImage previous_box = extract_box(last_screenshot, m_boxes[c]);
            QImage current_box = extract_box(current_screenshot, m_boxes[c]);

            FloatPixel current_average = pixel_average(current_box);
            double delta = image_diff_total(current_box, previous_box);

            double sigma = 0;
            if (stats.count() >= 5){
                sigma = stats.diff_metric(delta);
//                cout << sigma << endl;
            }

            double stddev = current_average.stddev();
            double brightness = current_average.sum();
            double average_euclidean_diff = pixel_average(extract_box(baseline_diff, m_boxes[c])).r;

#if 0
            trackers[c].push(
                PurpleDetectionEntry{
                    stddev,
                    brightness,
                    average_euclidean_diff
                },
                now
            );
//            PurpleDetectionAccumulator delta = trackers[c].accumulate_all();
            const PurpleDetectionEntry& oldest = trackers[c].oldest();
            const PurpleDetectionEntry& newest = trackers[c].newest();
            PurpleDetectionAccumulator delta{oldest, newest};
#endif

            if (best_sigma <= sigma){
                best_index = c;
                best_euclidean = average_euclidean_diff;
                best_stddev = stddev;
                best_brightness = brightness;
//                max_diff_delta = delta;
                best_delta = delta;
                best_sigma = sigma;
            }

            bool required = true;
//            required &= stddev < absolute_stddev;
            required &= brightness >= min_brightness;
            required &= average_euclidean_diff >= min_euclidean;

            required &= delta / stddev >= min_delta_ratio;
            required &= sigma / stddev >= min_sigma_ratio;

            if (required){
                purple = true;
//                break;
            }else{
                trackers[c].push(delta, now);
            }

            if (best_sigma <= sigma && required == purple){
                best_index = c;
                best_euclidean = average_euclidean_diff;
                best_stddev = stddev;
                best_brightness = brightness;
//                max_diff_delta = delta;
                best_delta = delta;
                best_sigma = sigma;
            }
        }

        QString str = "BeamReader: column = " + QString::number(best_index);
//        str += ", stddev = " + QString::number(max_diff_stddev) + " (" + QString::number(max_diff_delta.delta_stddev()) + ")";
//        str += ", brightness = " + QString::number(max_diff_brightness) + " (" + QString::number(max_diff_delta.brightness_ratio()) + ")";
//        str += ", euclidean = " + QString::number(max_diff) + " (" + QString::number(max_diff_delta.delta_euclidean()) + ")";

        str += ", stddev = " + QString::number(best_stddev);
        str += ", brightness = " + QString::number(best_brightness);
        str += ", euclidean = " + QString::number(best_euclidean);
        str += ", delta = " + QString::number(best_delta);
        str += ", sigma = " + QString::number(best_sigma);

        if (purple){
            m_logger.log(str, "blue");
            m_logger.log("BeamReader(): Purple beam found!", "blue");
            if (save_screenshot){
                current_screenshot.save("PurpleBeam-" + QString::fromStdString(now_to_filestring()) + ".png");
            }
            return Detection::PURPLE;
        }else{
            m_logger.log(str, "purple");
        }

        if (low_stddev_flag && text_stddev > 100){
            m_logger.log("BeamReader(): No beam detected with text. Resetting.", "blue");
            return Detection::RED_ASSUMED;
        }
#endif

        last_screenshot = std::move(current_screenshot);
    }while (!throttler.end_iteration(env));

    return Detection::NO_DETECTION;
}




}
}
}


