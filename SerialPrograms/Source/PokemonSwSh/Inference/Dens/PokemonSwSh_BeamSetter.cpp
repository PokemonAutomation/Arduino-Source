/*  Beam Setter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *      Drop a wishing piece and determine if it is red or purple.
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/Tools/StatAccumulator.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/InferenceThrottler.h"
#include "CommonTools/Images/ImageTools.h"
#include "CommonTools/TrendInference/TimeWindowStatTracker.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh_BeamSetter.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


BeamSetter::BeamSetter(ProgramEnvironment& /*env*/, VideoStream& stream, ProControllerContext& context)
    // : m_env(env)
    : m_stream(stream)
    , m_context(context)
    , m_text_box0(stream.overlay(), {0.400, 0.825, 0.05, 0.05}, COLOR_RED)
    , m_text_box1(stream.overlay(), {0.250, 0.900, 0.05, 0.05}, COLOR_RED)
    , m_box(stream.overlay(), {0.10, 0.005, 0.8, 0.470}, COLOR_RED)
{
    for (size_t c = 0; c < 32; c++){
        m_boxes.emplace_back(0.10 + 0.025*c, 0.005, 0.025, 0.470);
    }
}




BeamSetter::Detection BeamSetter::run(
    bool save_screenshot,
    Milliseconds timeout,
    double min_brightness,
    double min_euclidean,
    double min_delta_ratio,
    double min_sigma_ratio
){
    //  Grab baseline image.
    VideoSnapshot baseline_image = m_stream.video().snapshot();
    if (!baseline_image){
        m_stream.log("BeamSetter(): Screenshot failed.", COLOR_PURPLE);
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

    InferenceThrottler throttler(timeout, std::chrono::milliseconds(50));

    VideoSnapshot last_screenshot = baseline_image;
    do{
        //  Take screenshot.
        VideoSnapshot current_screenshot = m_stream.video().snapshot();
        if (!current_screenshot){
            m_stream.log("BeamSetter(): Screenshot failed.", COLOR_PURPLE);
            return Detection::NO_DETECTION;
        }

        //  Text detection.
        double text_stddev = image_stddev(extract_box_reference(current_screenshot, m_text_box0)).sum();
        text_stddev = std::max(text_stddev, image_stddev(extract_box_reference(current_screenshot, m_text_box1)).sum());
        if (text_stddev < 10){
            low_stddev_flag = true;
        }

        ImageRGB32 baseline_diff = image_diff_greyscale(baseline_image, current_screenshot);
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

            ImageViewRGB32 previous_box = extract_box_reference(last_screenshot, m_boxes[c]);
            ImageViewRGB32 current_box = extract_box_reference(current_screenshot, m_boxes[c]);

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

        std::string str = "BeamReader: column = " + std::to_string(best_index);

        str += ", stddev = " + tostr_default(best_stddev);
        str += ", brightness = " + tostr_default(best_brightness);
        str += ", euclidean = " + tostr_default(best_euclidean);
        str += ", delta = " + tostr_default(best_delta);
        str += ", sigma = " + tostr_default(best_sigma);

        if (purple){
            m_stream.log(str, COLOR_BLUE);
            m_stream.log("BeamReader(): Purple beam found!", COLOR_BLUE);
            if (save_screenshot){
                current_screenshot->save("PurpleBeam-" + now_to_filestring() + ".png");
            }
            return Detection::PURPLE;
        }else{
            m_stream.log(str, COLOR_PURPLE);
        }

        if (low_stddev_flag && text_stddev > 100){
            m_stream.log("BeamReader(): No beam detected with text. Resetting.", COLOR_BLUE);
            return Detection::RED_ASSUMED;
        }

        last_screenshot = std::move(current_screenshot);
    }while (!throttler.end_iteration(m_context));

    return Detection::NO_DETECTION;
}




}
}
}


