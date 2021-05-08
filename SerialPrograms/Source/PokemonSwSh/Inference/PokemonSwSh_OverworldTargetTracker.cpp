/*  Overworld Mark Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_MarkFinder.h"
#include "PokemonSwSh_OverworldTargetTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const double OverworldTargetTracker::OVERWORLD_CENTER_X = 0.50;
const double OverworldTargetTracker::OVERWORLD_CENTER_Y = 0.70;


OverworldTargetTracker::OverworldTargetTracker(
    Logger& logger, VideoFeed& feed,
    std::chrono::milliseconds window,
    double mark_offset,
    bool prioritize_exclamations,
    double max_alpha
)
    : m_logger(logger)
    , m_feed(feed)
    , m_window(window)
    , m_mark_offset(mark_offset)
    , m_prioritize_exclamations(prioritize_exclamations)
    , m_max_alpha(max_alpha)
    , m_search_area(feed, 0.0, 0.2, 1.0, 0.8)
    , m_stop_on_target(false)
{
    m_best_target.first = -1;
}

void OverworldTargetTracker::set_stop_on_target(bool stop){
    m_stop_on_target.store(stop, std::memory_order_release);
}
void OverworldTargetTracker::clear_detections(){
    SpinLockGuard lg(m_lock, "OverworldTargetTracker::clear_detections()");
    m_best_target.first = -1;
    m_exclamations.clear();
    m_questions.clear();
}
std::pair<double, OverworldTarget> OverworldTargetTracker::best_target(){
    SpinLockGuard lg(m_lock, "OverworldTargetTracker::best_target()");
    return m_best_target;
}


void OverworldTargetTracker::populate_targets(
    std::multimap<double, OverworldTarget>& scored_targets,
    const std::vector<OverworldTarget>& targets
){
#if 0
    cout << "Targets:" << endl;
    for (const auto& item : targets){
        cout << "    " << item.box.x << " - " << item.box.x + item.box.width
             << " x " << item.box.y << " - " << item.box.y + item.box.height << endl;
    }
#endif

//    cout << "Candidates:" << endl;
    for (size_t c = 0; c < targets.size(); c++){
        double overlap = 0;
        const InferenceBox& box0 = targets[c].box;
        for (size_t i = 0; i < targets.size(); i++){
            const InferenceBox& box1 = targets[i].box;
            double min_x = std::max(box0.x, box1.x);
            double max_x = std::min(box0.x + box0.width, box1.x + box1.width);
            if (min_x >= max_x){
                continue;
            }
            double min_y = std::max(box0.y, box1.y);
            double max_y = std::min(box0.y + box0.height, box1.y + box1.height);
            if (min_y >= max_y){
                continue;
            }
            overlap += (max_x - min_x) * (max_y - min_y);
        }
        double score = targets[c].trajectory.distance_in_ticks / overlap;
        scored_targets.emplace(score, targets[c]);
//        cout << "    " << score << " = "
//             << (int)targets[c].trajectory.joystick_x << ", "
//             << (int)targets[c].trajectory.joystick_y << endl;
    }
}

void OverworldTargetTracker::populate_targets(
    std::multimap<double, OverworldTarget>& scored_targets,
    const std::deque<Mark>& marks,
    OverworldMark mark
){
    std::vector<OverworldTarget> targets;
    for (const Mark& item : marks){
        const InferenceBox& box = item.box;
        double delta_x = box.x + box.width / 2 - OVERWORLD_CENTER_X;
        double delta_y = box.y + box.height * (1.0 + m_mark_offset) - OVERWORLD_CENTER_Y;
        Trajectory trajectory = get_trajectory_float(delta_x, delta_y);
        targets.emplace_back(OverworldTarget{mark, box, trajectory, delta_x, delta_y});
    }
    populate_targets(scored_targets, targets);
}

bool OverworldTargetTracker::save_target(std::multimap<double, OverworldTarget>::iterator target){
#if 0
    m_logger.log(
        QString("Best Target: ") +
        (target->second.mark == OverworldMark::EXCLAMATION_MARK ? "Exclamation" : "Question") +
        " at [" +
        QString::number(target->second.delta_x) + " , " +
        QString::number(-target->second.delta_y) + "], alpha = " +
        QString::number(target->first),
        "purple"
    );
#endif
//    SpinLockGuard lg(m_lock, "OverworldTargetTracker::save_target()");
    m_best_target = *target;
    return target->first <= m_max_alpha && m_stop_on_target.load(std::memory_order_acquire);
}

bool OverworldTargetTracker::on_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    QImage image = extract_box(frame, m_search_area);

    std::vector<PixelBox> exclamation_marks;
    std::vector<PixelBox> question_marks;
    find_marks(
        image,
        &exclamation_marks,
        &question_marks
    );


    SpinLockGuard lg(m_lock, "OverworldTargetTracker::on_frame()");

    //  Clear out old detections.
    auto oldest = timestamp - m_window;
    while (!m_exclamations.empty() && m_exclamations[0].timestamp < oldest){
        m_exclamations.pop_front();
    }
    while (!m_questions.empty() && m_questions[0].timestamp < oldest){
        m_questions.pop_front();
    }


    m_detection_boxes.clear();
    for (const PixelBox& mark : exclamation_marks){
        InferenceBox box = translate_to_parent(frame, m_search_area, mark);
        box.color = Qt::magenta;
        box.x -= box.width;
        box.width *= 3;
        box.height *= 1.5;
        m_exclamations.emplace_back(Mark{timestamp, box});
        m_detection_boxes.emplace_back(m_feed, box);
//        cout << "asdf = " << exclamations.size() << endl;
    }
    for (const PixelBox& mark : question_marks){
        InferenceBox box = translate_to_parent(frame, m_search_area, mark);
        box.color = Qt::magenta;
        box.x -= box.width / 2;
        box.width *= 2;
        box.height *= 1.5;
        m_questions.emplace_back(Mark{timestamp, box});
        m_detection_boxes.emplace_back(m_feed, box);
//        cout << "qwer = " << questions.size() << endl;
    }


    //  Build targets.

    if (!m_prioritize_exclamations){
        std::multimap<double, OverworldTarget> targets;
        populate_targets(targets, m_exclamations, OverworldMark::EXCLAMATION_MARK);
        populate_targets(targets, m_questions, OverworldMark::QUESTION_MARK);
        if (!targets.empty()){
            return save_target(targets.begin());
        }
    }else{
        std::multimap<double, OverworldTarget> exclamation_targets;
        std::multimap<double, OverworldTarget> question_targets;
        populate_targets(exclamation_targets, m_exclamations, OverworldMark::EXCLAMATION_MARK);
        populate_targets(question_targets, m_questions, OverworldMark::QUESTION_MARK);

        auto target0 = exclamation_targets.begin();
        auto target1 = question_targets.begin();

        //  See if we have any good target.
        if (!exclamation_targets.empty() && target0->first <= m_max_alpha){
            return save_target(target0);
        }
        if (!question_targets.empty() && target1->first <= m_max_alpha){
            return save_target(target1);
        }

        //  No good targets. Pick the next best one for logging purposes.
        std::multimap<double, OverworldTarget> targets;
        if (!exclamation_targets.empty()){
            targets.emplace(target0->first, target0->second);
        }
        if (!question_targets.empty()){
            targets.emplace(target1->first, target1->second);
        }
        if (!targets.empty()){
            return save_target(targets.begin());
        }
    }

    m_best_target.first = -1;
    return false;
}



}
}
}

