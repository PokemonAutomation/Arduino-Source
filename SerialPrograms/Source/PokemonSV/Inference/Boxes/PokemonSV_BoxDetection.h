/*  Box Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BoxDetection_H
#define PokemonAutomation_PokemonSV_BoxDetection_H

#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV/Inference/PokemonSV_WhiteButtonDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{


//  Detect whether the cursor is over a Pokemon or egg in the box.
//  It detects the yellow title bar on top-right corner of the screen when the cursor is on a pokemon or egg
class SomethingInBoxSlotDetector : public StaticScreenDetector{
public:
    SomethingInBoxSlotDetector(Color color, bool true_if_exists = true);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    bool m_true_if_exists;
    Color m_color;
    ImageFloatBox m_right;
    ImageFloatBox m_top;
    ImageFloatBox m_bottom;
    ImageFloatBox m_body;
};
class SomethingInBoxSlotWatcher : public DetectorToFinder<SomethingInBoxSlotDetector>{
public:
    SomethingInBoxSlotWatcher(Color color, bool stop_on_exists)
         : DetectorToFinder("SomethingInBoxSlot", std::chrono::milliseconds(250), color, stop_on_exists)
    {}
};



//  Detect whether you have a Pokemon with its menu open in the box system.
class BoxSelectDetector : public StaticScreenDetector{
public:
    BoxSelectDetector(Color color);

    // If there is a pokemon/egg in the current slot in box or party, while in box system view
    bool exists(const ImageViewRGB32& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    SomethingInBoxSlotDetector m_exists;
    AdvanceDialogDetector m_dialog;
    GradientArrowDetector m_gradient;
};
class BoxSelectWatcher : public DetectorToFinder<BoxSelectDetector>{
public:
    BoxSelectWatcher(
        Color color,
        std::chrono::milliseconds duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("BoxSelectFinder", duration, color)
    {}
};




enum class BoxCursorLocation{
    NONE,
    PARTY,      // player's party
    BOX_CHANGE, // on the box title bar
    ALL_BOXES,  // bottom-middle "All Boxes" button
    SEARCH,     // bottom-right "Search" button
    SLOTS,      // one of the 5 x 6 slots in the box
};
std::string BOX_CURSOR_LOCATION_NAMES(BoxCursorLocation location);

std::string BOX_LOCATION_STRING(BoxCursorLocation location, uint8_t row, uint8_t col);

struct BoxCursorCoordinates{
    uint8_t row;
    uint8_t col;
};

// Detect if the game is in box system view
class BoxDetector : public StaticScreenDetector{
public:
    BoxDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;
    std::pair<BoxCursorLocation, BoxCursorCoordinates> detect_location(const ImageViewRGB32& screen);

    //  While in the box system view, move the cursor to the desired slot.
    void move_cursor(
        const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
        BoxCursorLocation side, uint8_t row, uint8_t col
    );

private:
    bool to_coordinates(int& x, int& y, BoxCursorLocation side, uint8_t row, uint8_t col) const;
    void move_vertical(ProControllerContext& context, int current, int desired) const;
    void move_horizontal(ProControllerContext& context, int current, int desired) const;

private:
    Color m_color;
    GradientArrowDetector m_party;
    GradientArrowDetector m_box_change;
    GradientArrowDetector m_all_boxes;
    GradientArrowDetector m_search;
    GradientArrowDetector m_slots;
};
class BoxWatcher : public DetectorToFinder<BoxDetector>{
public:
    BoxWatcher(Color color)
         : DetectorToFinder("BoxFinder", std::chrono::milliseconds(250), color)
    {}
};

// Detect whether a box slot is empty by checking color stddev on that slot
// Note: due to the very slow loading of sprites in Pokemon SV box system, you need to make sure the sprites
// are fully loaded before calling this detector.
class BoxEmptySlotDetector : public StaticScreenDetector{
public:
    BoxEmptySlotDetector(BoxCursorLocation side, uint8_t row, uint8_t col, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;
private:
    Color m_color;
    ImageFloatBox m_box;
};
class BoxEmptySlotWatcher : public DetectorToFinder<BoxEmptySlotDetector>{
public:
    BoxEmptySlotWatcher(BoxCursorLocation side, uint8_t row, uint8_t col, FinderType finder_type = FinderType::PRESENT, Color color = COLOR_RED)
         : DetectorToFinder("BoxEmptySlotWatcher", finder_type, std::chrono::milliseconds(100), side, row, col, color)
    {}
};


// Detect party empty slots (the five slots after the party lead). Useful for egg hatching.
class BoxEmptyPartyWatcher : public VisualInferenceCallback{
public:
    BoxEmptyPartyWatcher(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    
    // Return true when the watcher is sure that each of the five slots is either egg, non-egg pokemon or empty.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    uint8_t num_empty_slots_found() const;

private:
    FixedLimitVector<BoxEmptySlotWatcher> m_empty_watchers;
};


// Detect whether there is a button Y in the bottom row of the box system view
class BoxBottomButtonYDetector : public WhiteButtonDetector{
public:
    BoxBottomButtonYDetector(Color color = COLOR_RED);
};

// Detect whether there is a button B in the bottom row of the box system view
class BoxBottomButtonBDetector : public WhiteButtonDetector{
public:
    BoxBottomButtonBDetector(Color color = COLOR_RED);
};

// Detect whether we are in box selection mode or not
class BoxSelectionBoxModeWatcher : public VisualInferenceCallback{
public:
    BoxSelectionBoxModeWatcher(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    
    // Return true when the watcher is sure that we are either in box selection mode or not
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    // Return whether in box selection mode
    bool in_box_selection_mode() const;

private:
    WhiteButtonWatcher button_y_watcher;
};




}
}
}
#endif
