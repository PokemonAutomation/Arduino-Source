/*  Overworld Target Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_OverworldTargetTracker_H
#define PokemonAutomation_PokemonSwSh_OverworldTargetTracker_H

#include <vector>
#include <deque>
#include <map>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonSwSh/Programs/OverworldBot/PokemonSwSh_OverworldTrajectory.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



enum class OverworldMark{
    EXCLAMATION_MARK,
    QUESTION_MARK,
};

enum class MarkPriority{
    EXCLAMATION_ONLY,
    PRIORITIZE_EXCLAMATION,
    NO_PREFERENCE,
    PRIORITIZE_QUESTION,
    QUESTION_ONLY,
};
extern const std::vector<std::string> MARK_PRIORITY_STRINGS;

struct OverworldTarget{
    OverworldMark mark;
    ImageFloatBox box;
    Trajectory trajectory;
    double delta_x;
    double delta_y;
};


class OverworldTargetTracker : public VisualInferenceCallback{
public:
    static const double OVERWORLD_CENTER_X;
    static const double OVERWORLD_CENTER_Y;

public:
    OverworldTargetTracker(
        Logger& logger, VideoOverlay& overlay,
        std::chrono::milliseconds window,
        double mark_offset,
        MarkPriority mark_priority,
        double max_alpha
    );

    //  If set to true, this inference object will not return true on
    //  "on_frame()" callbacks.
    void set_stop_on_target(bool stop);

    void clear_detections();

    bool has_good_target();

    //  Get the best target as of right now.
    //  The return value is only valid if the first element is non-negative.
    std::pair<double, OverworldTarget> best_target() const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override final;


private:
    struct Mark{
        WallClock timestamp;
        ImageFloatBox box;
    };

    static void populate_targets(
        std::multimap<double, OverworldTarget>& scored_targets,
        const std::vector<OverworldTarget>& targets
    );
    void populate_targets(
        std::multimap<double, OverworldTarget>& scored_targets,
        const std::deque<Mark>& marks,
        OverworldMark mark
    );

    bool save_target(std::multimap<double, OverworldTarget>::iterator target);


private:
    Logger& m_logger;
    VideoOverlay& m_overlay;
    std::chrono::milliseconds m_window;
    double m_mark_offset;
    MarkPriority m_mark_priority;
    double m_max_alpha;

    ImageFloatBox m_search_area;
    std::deque<OverlayBoxScope> m_detection_boxes;

    //  Sliding window of detections.
    std::deque<Mark> m_exclamations;
    std::deque<Mark> m_questions;

    std::atomic<bool> m_stop_on_target;
    mutable SpinLock m_lock;
    std::pair<double, OverworldTarget> m_best_target;
};





}
}
}
#endif
