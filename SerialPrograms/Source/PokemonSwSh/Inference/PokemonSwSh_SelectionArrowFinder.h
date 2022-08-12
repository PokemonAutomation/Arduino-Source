/*  Selection Arrow Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SelectionArrowFinder_H
#define PokemonAutomation_PokemonSwSh_SelectionArrowFinder_H

#include <atomic>
#include <deque>
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class SelectionArrowFinder : public VisualInferenceCallback{
public:
    SelectionArrowFinder(VideoOverlay& overlay, const ImageFloatBox& box);

    bool detect(const ImageViewRGB32& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

protected:
    VideoOverlay& m_overlay;
    ImageFloatBox m_box;
    std::deque<InferenceBoxScope> m_arrow_boxes;
};





class BattleMoveArrowFinder : public SelectionArrowFinder{
public:
//    static const ImageFloatBox BATTLE_MOVE_SELECTION;

public:
    BattleMoveArrowFinder(VideoOverlay& overlay);

    int8_t get_slot();

    //  These are not thread safe.
    int8_t detect(const ImageViewRGB32& screen);
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;


private:
    int8_t arrow_slot(double y_center);


private:
    std::atomic<int8_t> m_arrow_slot;
};



}
}
}
#endif
