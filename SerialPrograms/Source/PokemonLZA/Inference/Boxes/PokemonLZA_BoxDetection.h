/*  Box Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BoxDetection_H
#define PokemonAutomation_PokemonLZA_BoxDetection_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"

#include <vector>

namespace PokemonAutomation{
    struct ProgramInfo;
    class VideoStream;
    template <typename Type> class ControllerContext;
    class VideoOverlay;
    class VideoOverlaySet;
namespace NintendoSwitch{
    class ProController;
    using ProControllerContext = ControllerContext<ProController>;
namespace PokemonLZA{


// The box view has one row for current player party of 6 pokemon and 5 row x 6 col box below it.
// row == 0 means the cursor is at player party while row >= 1 means the cursor is at current box
struct BoxCursorCoordinates{
    uint8_t row;
    uint8_t col;
    const static uint8_t INVALID = 255;
};


// Detect if the game is in box system view.
// Also handle moving cursor to target slot in box system view
class BoxDetector : public StaticScreenDetector{
public:
    BoxDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    // return detected location fround by calling `detect()`
    BoxCursorCoordinates detected_location() const;

    // While in the box system view, move the cursor to the desired slot.
    void move_cursor(
        const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
        uint8_t row, uint8_t col
    );

    // Under debug mode, will throw FatalProgramException when more than one box cell
    // detects a cursor. This is used for debugging.
    void set_debug_mode(bool debug_mode) { m_debug_mode = debug_mode; }

private:
    // called for each box cell to check if the selection arrow is above that cell
    bool detect_at_cell(const ImageViewRGB32& image_crop);

    Color m_color;

    ButtonDetector m_plus_button;
    std::vector<ImageFloatBox> m_arrow_boxes;  // all 6 x 6 potential locations of the arrow interiors on box view
    uint8_t m_found_row = 0;
    uint8_t m_found_col = 0;
    bool m_debug_mode = false;
};

class BoxWatcher : public DetectorToFinder<BoxDetector>{
public:
    BoxWatcher(Color color = COLOR_RED, VideoOverlay* overlay = nullptr)
         : DetectorToFinder("BoxWatcher", std::chrono::milliseconds(250), color, overlay)
    {}
};


//  Detect whether the cursor is over a Pokemon or an empty cell in the box.
//  It detects the right stick up and down icon on the right edge of the screen
class SomethingInBoxCellDetector : public StaticScreenDetector{
public:
    SomethingInBoxCellDetector(Color color, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ButtonDetector m_right_stick_up_down_detector;
};
class SomethingInBoxCellWatcher : public DetectorToFinder<SomethingInBoxCellDetector>{
public:
    SomethingInBoxCellWatcher(Color color, VideoOverlay* overlay = nullptr)
         : DetectorToFinder("SomethingInBoxCell", FinderType::CONSISTENT, std::chrono::milliseconds(250), color, overlay)
    {}
};



}
}
}
#endif
