/*  Selection Arrow Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SelectionArrowFinder_H
#define PokemonAutomation_PokemonSwSh_SelectionArrowFinder_H

#include <atomic>
#include <deque>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

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
    std::deque<OverlayBoxScope> m_arrow_boxes;
};



class BattleMoveArrowFinder : public SelectionArrowFinder{
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


// Selection arrow for the dialog of the egg lady when you would like to retrieve an egg.
class RetrieveEggArrowFinder : public SelectionArrowFinder{
public:
    RetrieveEggArrowFinder(VideoOverlay& overlay);
};


// Selection arrow for the dialog of the egg lady when you check the condition of the pokemon
// in the Pokemon Nursery, where there is no egg.
class CheckNurseryArrowFinder : public SelectionArrowFinder{
public:
    CheckNurseryArrowFinder(VideoOverlay& overlay);
};


// Selection arrow for the menu of the pokemon in a pokemon storage box.
// The menuitems are: "Move", "Check summary", "Check held item", "Change markings", ...
class StoragePokemonMenuArrowFinder : public SelectionArrowFinder{
public:
    StoragePokemonMenuArrowFinder(VideoOverlay& overlay);
};

// The arrow that points to one of the ten apps on Rotom Phone menu
class RotomPhoneMenuArrowFinder : public StaticScreenDetector{
public:
    RotomPhoneMenuArrowFinder(VideoOverlay& overlay);

    int current_index() const{
        return m_index;
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    // Detect which app is selected by the arrow. Return the index of the app
    // The order is: from top to bottom, from left to right.
    // If no arrow found, return -1.
    int detect_index(const ImageViewRGB32& screen);

private:
    VideoOverlaySet m_overlay_set;
    int m_index = -1;
};
class RotomPhoneMenuArrowWatcher : public DetectorToFinder<RotomPhoneMenuArrowFinder>{
public:
    RotomPhoneMenuArrowWatcher(VideoOverlay& overlay)
         : DetectorToFinder("RotomPhoneMenuArrowWatcher", std::chrono::milliseconds(250), overlay)
    {}
};



}
}
}
#endif
