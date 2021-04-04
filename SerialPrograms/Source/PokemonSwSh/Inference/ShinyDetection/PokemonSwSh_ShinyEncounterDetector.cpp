/*  Shiny Encounter Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <deque>
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/Inference/StatAccumulator.h"
#include "CommonFramework/Inference/TimeWindowStatTracker.h"
#include "CommonFramework/Inference/InferenceThrottler.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh_ShinyTrigger.h"
#include "PokemonSwSh_ShinyEncounterDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const ShinyDetectionBattle SHINY_BATTLE_REGULAR {{0.5, 0.05, 0.5, 0.70}, std::chrono::milliseconds(2300)};
const ShinyDetectionBattle SHINY_BATTLE_RAID    {{0.3, 0.01, 0.7, 0.75}, std::chrono::milliseconds(3900)};



class ShinyEncounterDetector{
public:
    enum class DialogTimerState{
        NOT_STARTED,
        LISTENING,
        FINISHED,
    };

public:
    ShinyEncounterDetector(
        VideoFeed& feed, Logger& logger,
        const ShinyDetectionBattle& battle_settings,
        double detection_threshold
    );

    ShinyDetection results() const;

    void push(
        const QImage& screen,
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now()
    );


private:
    VideoFeed& m_feed;
    Logger& m_logger;

    InferenceBoxScope m_dialog_box;
    InferenceBoxScope m_shiny_box;

    std::chrono::milliseconds m_min_delay;
    std::chrono::milliseconds m_max_delay;

    double m_detection_threshold;
    StandardBattleMenuDetector m_menu;

    bool m_dialog_on = false;
    DialogTimerState m_dialog_state = DialogTimerState::NOT_STARTED;

    std::chrono::system_clock::time_point m_last_dialog_off;
    ShinyImageAlpha m_dialog_alpha;

    std::deque<InferenceBoxScope> m_detection_overlays;

    bool m_dialog_trigger = false;
    ShinyImageAlpha m_image_alpha;

};




ShinyEncounterDetector::ShinyEncounterDetector(
    VideoFeed& feed, Logger& logger,
    const ShinyDetectionBattle& battle_settings,
    double detection_threshold
)
    : m_feed(feed)
    , m_logger(logger)
    , m_dialog_box(feed, 0.50, 0.89, 0.40, 0.07)
    , m_shiny_box(feed, battle_settings.detection_box)
    , m_min_delay(battle_settings.dialog_delay_when_shiny - std::chrono::milliseconds(500))
    , m_max_delay(battle_settings.dialog_delay_when_shiny + std::chrono::milliseconds(500))
    , m_detection_threshold(detection_threshold)
    , m_menu(feed)
    , m_last_dialog_off(std::chrono::system_clock::now())
{}


void ShinyEncounterDetector::push(
    const QImage& screen,
    std::chrono::system_clock::time_point timestamp
){
    //  Dialog timing detector.
    bool dialog_on = is_dialog_grey(extract_box(screen, m_dialog_box));
    do{
        if (m_dialog_on){
            if (dialog_on){
                //  on -> on    :   Zero the alpha.
                m_dialog_alpha = ShinyImageAlpha();
            }else{
                //  on -> off   :   Reset the timer.
                if (m_dialog_state != DialogTimerState::FINISHED){
                    m_dialog_state = DialogTimerState::LISTENING;
                }
                m_last_dialog_off = timestamp;
                m_dialog_on = false;
                m_logger.log("ShinyDetector: Dialog on -> off.", "purple");
            }
            break;
        }

        if (!dialog_on){
            break;
        }
        if (m_dialog_state == DialogTimerState::NOT_STARTED){
                m_logger.log("ShinyDetector: Dialog off -> on. Starting timer.", "purple");
            break;
        }

        //  off -> on   :   Check if we sandwiched a shiny sparkle with the appropriate time delay.
        auto time_off = timestamp - m_last_dialog_off;
        bool shiny_delay = m_min_delay < time_off && time_off < m_max_delay;
        bool sandwiched_shiny = m_dialog_alpha.shiny >= 0.5;
        bool dialog_trigger = shiny_delay && sandwiched_shiny;

        m_logger.log(
            "ShinyDetector: Dialog Delay = " +
            QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(time_off).count() / 1000.) + " seconds",
            "purple"
        );
        if (dialog_trigger){
            if (m_dialog_state == DialogTimerState::LISTENING){
                m_logger.log("ShinyDetector: Dialog delay is consistent with a shiny!", "blue");
                m_dialog_trigger = true;
            }else{
                m_logger.log("ShinyDetector: Dialog delay is consistent with a shiny, but it's the wrong dialog.", "blue");
            }
        }

        m_dialog_state = DialogTimerState::FINISHED;
    }while (false);
    m_dialog_on = dialog_on;




    QImage shiny_box = extract_box(screen, m_shiny_box);

    ShinyImageDetection signatures;
    signatures.accumulate(shiny_box, timestamp.time_since_epoch().count(), &m_logger);

    ShinyImageAlpha frame_alpha = signatures.alpha();
    m_image_alpha.max(frame_alpha);

    if (!m_dialog_on){
        m_dialog_alpha.max(frame_alpha);
    }


    if (frame_alpha.shiny > 0){
        if (frame_alpha.shiny > m_detection_threshold){
            m_logger.log(
                "ShinyDetector: alpha = " + QString::number(frame_alpha.shiny) + " / "  + QString::number(m_image_alpha.shiny) + " (threshold exceeded)",
                "blue"
            );
        }else{
            m_logger.log(
                "ShinyDetector: alpha = " + QString::number(frame_alpha.shiny) + " / "  + QString::number(m_image_alpha.shiny),
                "blue"
            );
        }
    }

    m_detection_overlays.clear();
    for (const auto& item : signatures.balls){
        InferenceBox box = translate_to_parent(screen, m_shiny_box, item);
        box.color = Qt::green;
        m_detection_overlays.emplace_back(m_feed, box);
    }
    for (const auto& item : signatures.stars){
        InferenceBox box = translate_to_parent(screen, m_shiny_box, item);
        box.color = Qt::green;
        m_detection_overlays.emplace_back(m_feed, box);
    }
    for (const auto& item : signatures.squares){
        InferenceBox box = translate_to_parent(screen, m_shiny_box, item);
        box.color = Qt::green;
        m_detection_overlays.emplace_back(m_feed, box);
    }
    for (const auto& item : signatures.lines){
        InferenceBox box = translate_to_parent(screen, m_shiny_box, item);
        box.color = Qt::green;
        m_detection_overlays.emplace_back(m_feed, box);
    }
}

ShinyDetection ShinyEncounterDetector::results() const{
    double alpha = m_image_alpha.shiny;
    if (m_dialog_trigger){
        alpha += 1.0;
    }
    m_logger.log(
        "ShinyDetector: Overall Alpha = " + QString::number(alpha) +
        ", Star Alpha = " + QString::number(m_image_alpha.star) +
        ", Square Alpha = " + QString::number(m_image_alpha.square),
        "purple"
    );

    if (alpha < m_detection_threshold){
        m_logger.log("ShinyDetector: Not Shiny.", "purple");
        return ShinyDetection::NOT_SHINY;
    }
    if (m_image_alpha.star > 0 && m_image_alpha.star > m_image_alpha.square * 2){
        m_logger.log("ShinyDetector: Detected Star Shiny!", "blue");
        return ShinyDetection::STAR_SHINY;
    }
    if (m_image_alpha.square > 0 && m_image_alpha.square > m_image_alpha.star * 2){
        m_logger.log("ShinyDetector: Detected Square Shiny!", "blue");
        return ShinyDetection::SQUARE_SHINY;
    }

    m_logger.log("ShinyDetector: Detected Shiny! But ambiguous shiny type.", "blue");
    return ShinyDetection::UNKNOWN_SHINY;
}




ShinyDetection detect_shiny_battle(
    ProgramEnvironment& env,
    VideoFeed& feed, Logger& logger,
    const ShinyDetectionBattle& battle_settings,
    std::chrono::seconds timeout,
    double detection_threshold
){
#if 0
    ShinyEncounterDetector detector(env, feed, logger, shiny_box, timeout, detection_threshold);
    return detector.detect();
#else

    StatAccumulatorI32 capture_stats;
    StatAccumulatorI32 menu_stats;
    StatAccumulatorI32 inference_stats;
    StatAccumulatorI32 throttle_stats;

    StandardBattleMenuDetector menu(feed);
    ShinyEncounterDetector detector(feed, logger, battle_settings, detection_threshold);

    bool no_detection = false;

    InferenceThrottler throttler(timeout, std::chrono::milliseconds(50));
    while (true){
        env.check_stopping();

        auto time0 = std::chrono::system_clock::now();
        QImage screen = feed.snapshot();
        if (screen.isNull()){

        }
        auto time1 = std::chrono::system_clock::now();
        capture_stats += std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count();
        auto timestamp = time1;

        if (menu.detect(screen)){
            logger.log("ShinyDetector: Battle menu found!", "purple");
            break;
        }
        auto time2 = std::chrono::system_clock::now();
        menu_stats += std::chrono::duration_cast<std::chrono::milliseconds>(time2 - time1).count();

        detector.push(screen, timestamp);
        auto time3 = std::chrono::system_clock::now();
        inference_stats += std::chrono::duration_cast<std::chrono::milliseconds>(time3 - time2).count();

//        if (time3 - time2 > std::chrono::milliseconds(50)){
//            screen.save("slow-inference.png");
//        }

        if (throttler.end_iteration(env)){
            no_detection = true;
            break;
        }
        auto time4 = std::chrono::system_clock::now();
        throttle_stats += std::chrono::duration_cast<std::chrono::milliseconds>(time4 - time3).count();
    }

    logger.log("Diagnostics: Screenshot:\r\n" + capture_stats.dump(), Qt::magenta);
    logger.log("Diagnostics: Menu Detection:\r\n" + menu_stats.dump(), Qt::magenta);
    logger.log("Diagnostics: Inference:\r\n" + inference_stats.dump(), Qt::magenta);
    logger.log("Diagnostics: Throttle:\r\n" + throttle_stats.dump(), Qt::magenta);

    if (no_detection){
        logger.log("ShinyDetector: Battle menu not found after timeout.", "red");
        return ShinyDetection::NO_BATTLE_MENU;
    }

    return detector.results();

#endif
}



}
}
}
