/*  Box Detection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BoxDetection_H
#define PokemonAutomation_PokemonSV_BoxDetection_H

#include "PokemonSV_DialogDetector.h"
#include "PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSV{


//  Detect whether the cursor if over a Pokemon in the box.
class SomethingInBoxSlotDetector : public StaticScreenDetector{
public:
    SomethingInBoxSlotDetector(Color color, bool true_if_exists);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    bool m_true_if_exists;
    Color m_color;
    ImageFloatBox m_exists;
};
class SomethingInBoxSlotWatcher : public DetectorToFinder<SomethingInBoxSlotDetector>{
public:
    SomethingInBoxSlotWatcher(Color color, bool stop_on_exists)
         : DetectorToFinder("SomethingInBoxSlot", std::chrono::milliseconds(250), color, stop_on_exists)
    {}
};



//  Detect whether you have a Pokemon selected in the box system.
class BoxSelectDetector : public StaticScreenDetector{
public:
    BoxSelectDetector(Color color);

    bool exists(const ImageViewRGB32& screen) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    SomethingInBoxSlotDetector m_exists;
    AdvanceDialogDetector m_dialog;
    GradientArrowDetector m_gradient;
};
class BoxSelectWatcher : public DetectorToFinder<BoxSelectDetector>{
public:
    BoxSelectWatcher(Color color)
         : DetectorToFinder("BoxSelectFinder", std::chrono::milliseconds(250), color)
    {}
};




enum class BoxCursorLocation{
    NONE,
    PARTY,
    BOX_CHANGE,
    ALL_BOXES,
    SEARCH,
    SLOTS,
};
struct BoxCursorCoordinates{
    uint8_t row;
    uint8_t col;
};

class BoxDetector : public StaticScreenDetector{
public:
    BoxDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;
    std::pair<BoxCursorLocation, BoxCursorCoordinates> detect_location(const ImageViewRGB32& screen) const;

    //  While sitting on the menu, move the cursor to the desired slot.
    void move_cursor(
        ConsoleHandle& console, BotBaseContext& context,
        BoxCursorLocation side, uint8_t row, uint8_t col
    ) const;

private:
    bool to_coordinates(int& x, int& y, BoxCursorLocation side, uint8_t row, uint8_t col) const;
    void move_vertical(BotBaseContext& context, int current, int desired) const;
    void move_horizontal(BotBaseContext& context, int current, int desired) const;

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






}
}
}
#endif
