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
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CommonFramework/Inference/TimeWindowStatTracker.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh_BeamSetter.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


BeamSetter::BeamSetter(ProgramEnvironment& /*env*/, ConsoleHandle& console, BotBaseContext& context)
    // : m_env(env)
    : m_console(console)
    , m_context(context)
    , m_text_box(console, 0.400, 0.825, 0.05, 0.05, COLOR_RED)
    , m_box(console, 0.10, 0.005, 0.8, 0.470, COLOR_RED)
{
    for (size_t c = 0; c < 32; c++){
        m_boxes.emplace_back(0.10 + 0.025*c, 0.005, 0.025, 0.470);
    }
}




BeamSetter::Detection BeamSetter::run(
    bool save_screenshot,
    uint16_t timeout_ticks,
    double min_brightness,
    double min_euclidean,
    double min_delta_ratio,
    double min_sigma_ratio
){
    //  Grab baseline image.
    QImage baseline_image = m_console.video().snapshot();
    if (baseline_image.isNull()){
        m_console.log("BeamSetter(): Screenshot failed.", COLOR_PURPLE);
        return Detection::NO_DETECTION;
    }

    std::vector<FloatPixel> baseline_values(m_boxes.size());
    std::vector<FloatPixel> baseline_ratios(m_boxes.size());
    for (size_t c = 0; c < m_boxes.size(); c++){
        baseline_values[c] = image_average(extract_box_reference(baseline_image, m_boxes[c]));
        baseline_ratios[c] = baseline_values[c] / baseline_values[c].sum();
    }

    //  Drop the wishing piece.
    pbf_press_button(m_context, BUTTON_A, 10, 10);
    m_context.wait_for_all_requests();

    bool low_stddev_flag = false;

    std::vector<TimeWindowStatTracker<FloatStatAccumulator>> trackers;
    for (size_t c = 0; c < m_boxes.size(); c++){
        trackers.emplace_back(std::chrono::milliseconds(1000));
    }

    InferenceThrottler throttler(
        std::chrono::milliseconds((uint64_t)timeout_ticks * 1000 / TICKS_PER_SECOND),
        std::chrono::milliseconds(50)
    );

    QImage last_screenshot = baseline_image;
    do{
        //  Take screenshot.
        QImage current_screenshot = m_console.video().snapshot();
        if (current_screenshot.isNull()){
            m_console.log("BeamSetter(): Screenshot failed.", COLOR_PURPLE);
            return Detection::NO_DETECTION;
        }

        //  Text detection.
        double text_stddev = image_stddev(extract_box_reference(current_screenshot, m_text_box)).sum();
        if (text_stddev < 10){
            low_stddev_flag = true;
        }

        QImage baseline_diff = image_diff_greyscale(baseline_image, current_screenshot);
        auto now = current_time();

        bool purple = false;
        size_t best_index = 0;
        double best_euclidean = 0;
        double best_stddev = 0;
        double best_brightness = 0;
        double best_delta = 0;
        double best_sigma = 0;
        for (size_t c = 0; c < m_boxes.size(); c++){
            FloatStatAccumulator stats = trackers[c].accumulate_all();

            ConstImageRef previous_box = extract_box_reference(last_screenshot, m_boxes[c]);
            ConstImageRef current_box = extract_box_reference(current_screenshot, m_boxes[c]);

            FloatPixel current_average = image_average(current_box);
            double delta = ImageMatch::pixel_RMSD(current_box, previous_box);

            double sigma = 0;
            if (stats.count() >= 5){
                sigma = stats.diff_metric(delta);
            }

            double stddev = current_average.stddev();
            double brightness = current_average.sum();
            double average_euclidean_diff = image_average(extract_box_reference(baseline_diff, m_boxes[c])).r;

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
            required &= brightness >= min_brightness;
            required &= average_euclidean_diff >= min_euclidean;

            required &= delta / stddev >= min_delta_ratio;
            required &= sigma / stddev >= min_sigma_ratio;

            if (required){
                purple = true;
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

        std::string str = "BeamReader: column = " + tostr_default(best_index);

        str += ", stddev = " + tostr_default(best_stddev);
        str += ", brightness = " + tostr_default(best_brightness);
        str += ", euclidean = " + tostr_default(best_euclidean);
        str += ", delta = " + tostr_default(best_delta);
        str += ", sigma = " + tostr_default(best_sigma);

        if (purple){
            m_console.log(str, COLOR_BLUE);
            m_console.log("BeamReader(): Purple beam found!", COLOR_BLUE);
            if (save_screenshot){
                current_screenshot.save("PurpleBeam-" + QString::fromStdString(now_to_filestring()) + ".png");
            }
            return Detection::PURPLE;
        }else{
            m_console.log(str, COLOR_PURPLE);
        }

        if (low_stddev_flag && text_stddev > 100){
            m_console.log("BeamReader(): No beam detected with text. Resetting.", COLOR_BLUE);
            return Detection::RED_ASSUMED;
        }

        last_screenshot = std::move(current_screenshot);
    }while (!throttler.end_iteration(m_context));

    return Detection::NO_DETECTION;
}




}
}
}


