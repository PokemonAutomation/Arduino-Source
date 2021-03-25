/*  Beam Setter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Drop a wishing piece and determine if it is red or purple.
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_BeamSetter.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


BeamSetter::BeamSetter(VideoFeed& feed, Logger& logger)
    : m_feed(feed)
    , m_logger(logger)
    , m_text_box(feed, Qt::red, 0.400, 0.825, 0.05, 0.05)
    , m_box(feed, Qt::red, 0.10, 0.2, 0.8, 0.275)
{
    for (size_t c = 0; c < 32; c++){
        m_boxes.emplace_back(0.10 + 0.025*c, 0.2, 0.025, 0.275);
    }
}

BeamSetter::Detection BeamSetter::run(
    ProgramEnvironment& env, BotBase& botbase,
    double detection_threshold, uint16_t timeout_ticks
){
    //  Grab baseline image.
    QImage baseline_image = m_feed.snapshot();
    if (baseline_image.isNull()){
        m_logger.log("BeamSetter(): Screenshot failed.", "purple");
        return Detection::NO_DETECTION;
    }
//    baseline_image.save("f:/test0.jpg");
//    cout << "=======================" << endl;

    std::vector<FloatPixel> baseline_ratios(m_boxes.size());
    for (size_t c = 0; c < m_boxes.size(); c++){
        baseline_ratios[c] = pixel_average_normalized(extract_box(baseline_image, m_boxes[c]));
    }

    //  Drop the wishing piece.
    pbf_press_button(botbase, BUTTON_A, 10, 10);
    botbase.wait_for_all_requests();
    uint32_t start = system_clock(botbase);

    //  Set up detection history.
    std::map<size_t, size_t> red_detections;
    std::map<size_t, size_t> purple_detections;

    bool low_stddev_flag = false;
    std::vector<FloatPixel> current_values(m_boxes.size());
    std::vector<FloatPixel> current_ratio_diffs(m_boxes.size());
    uint32_t now = start;
    while (now - start < timeout_ticks){
        //  Take screenshot.
        QImage current = m_feed.snapshot();
        if (current.isNull()){
            m_logger.log("BeamSetter(): Screenshot failed.", "purple");
            return Detection::NO_DETECTION;
        }
//        current.save("f:/test1.jpg");

        //  Text detection.
        double text_stddev = pixel_stddev(extract_box(current, m_text_box)).sum();
        if (text_stddev < 10){
            low_stddev_flag = true;
        }

        //  Compute ratios.
        FloatPixel average_diff;
        for (size_t c = 0; c < m_boxes.size(); c++){
            current_values[c] = pixel_average(extract_box(current, m_boxes[c]));
            FloatPixel ratio = current_values[c] / current_values[c].sum();
            current_ratio_diffs[c] = ratio - baseline_ratios[c];
//            cout << c << " - " << current_ratio_diffs[c] << endl;
            average_diff += current_ratio_diffs[c];
        }
        average_diff /= m_boxes.size();

        //  Detect all columns.
        double max_red_diff = -1.0;
        size_t max_red_diff_index = 0;
        double min_stddev = 255;
        size_t min_stddev_index = 0;
        for (size_t c = 0; c < m_boxes.size(); c++){
            FloatPixel diff = current_ratio_diffs[c] - average_diff;
//                cout << "current_values[" << c << "] = " << current_values[c] << ", " << diff << endl;
            double stddev = current_values[c].stddev();
            if (max_red_diff < diff.r){
                max_red_diff = diff.r;
                max_red_diff_index = c;
            }
            if (min_stddev > stddev){
                min_stddev = stddev;
                min_stddev_index = c;
            }
            if (stddev < 10 && current_values[c].sum() > 500){
                size_t& count = purple_detections[c];
                count++;
            }
            if (diff.r > detection_threshold){
                size_t& count = red_detections[c];
                count++;
            }
        }

        QString str =
            "BeamReader(): r[" + QString::number(max_red_diff_index) + "] = " +
            QString::number(current_ratio_diffs[max_red_diff_index].r - average_diff.r) +
            ", b[" + QString::number(min_stddev_index) + "] = " + QString::number(min_stddev) +
            ", t = " + QString::number(text_stddev);
        m_logger.log(str, "purple");

        if (!red_detections.empty()){
            str = "BeamReader(): Red = ";
            size_t count = 0;
            for (const auto& column : red_detections){
                count = std::max(count, column.second);
                str += "[" + QString::number(column.second) + " x " +
                    QString::number(column.first) + "-" +
                    QString::number(current_ratio_diffs[column.first].r - average_diff.r) + "]";
            }
            m_logger.log(str, "purple");
            if (count >= 5){
                m_logger.log("BeamReader(): 5 positive red reads. Red beam found.", "blue");
                return Detection::RED_DETECTED;
            }
        }
        if (!purple_detections.empty()){
            str = "BeamReader(): Purple = ";
            size_t count = 0;
            for (const auto& column : purple_detections){
                count = std::max(count, column.second);
                str += "[" + QString::number(column.second) + " x " +
                    QString::number(column.first) + "-" +
                    current_values[column.first].to_string() + "]";
            }
            m_logger.log(str, "purple");
            if (count >= 1){
                m_logger.log("BeamReader(): Purple beam found!", "blue");
                return Detection::PURPLE;
            }
        }
        if (low_stddev_flag && text_stddev > 100){
            m_logger.log("BeamReader(): No beam detected with text. Resetting.", "blue");
            return Detection::RED_ASSUMED;
        }

        env.wait(std::chrono::milliseconds(50));
        now = system_clock(botbase);
    }
    return Detection::NO_DETECTION;
}




}
}
}


