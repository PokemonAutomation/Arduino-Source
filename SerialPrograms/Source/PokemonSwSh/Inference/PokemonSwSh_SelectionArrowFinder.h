/*  Selection Arrow Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SelectionArrowFinder_H
#define PokemonAutomation_PokemonSwSh_SelectionArrowFinder_H

#include <deque>
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class SelectionArrowFinder : public VisualInferenceCallback{
public:
    SelectionArrowFinder(VideoOverlay& overlay, const ImageFloatBox& box);

    bool detect(const QImage& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;

protected:
    VideoOverlay& m_overlay;
    ImageFloatBox m_box;
    std::deque<InferenceBoxScope> m_arrow_boxes;
};





class BattleMoveArrowFinder : public SelectionArrowFinder{
public:
    static const ImageFloatBox BATTLE_MOVE_SELECTION;

public:
    BattleMoveArrowFinder(VideoOverlay& overlay);

    int8_t get_slot();

    //  These are not thread safe.
    int8_t detect(const QImage& screen);
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;


private:
    int8_t arrow_slot(double y_center);


private:
    std::atomic<int8_t> m_arrow_slot;
};



}
}
}
#endif
