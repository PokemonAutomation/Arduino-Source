/*  Selection Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_SelectionArrowDetector_H
#define PokemonAutomation_PokemonHome_SelectionArrowDetector_H

#include <optional>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


enum class BoxSelectionMode{
    Normal,     // Red
    Relocate,   // Blue
    Multi,      // Green
};

enum class BoxSelectionLocation{
    Grid,
    BoxName,
    BoxSpaces,
    Newest30,
};

struct BoxSelection{
    BoxSelectionMode mode;
    BoxSelectionLocation location;
    // Zero-based coordinates in the 5-row, 6-column grid. Only valid for Grid.
    size_t row = 0;
    size_t column = 0;
};

class SelectionArrowDetector{
public:
    SelectionArrowDetector(Color color = COLOR_RED) : m_color(color) {}

    void make_overlays(VideoOverlaySet& items) const;
    // Returns no value when no arrow matches a supported box selection position.
    std::optional<BoxSelection> detect(const ImageViewRGB32& screen) const;

private:
    Color m_color;
};

class SelectionArrowWatcher : public SelectionArrowDetector, public VisualInferenceCallback{
public:
    SelectionArrowWatcher(Color color, BoxSelectionMode mode, BoxSelectionLocation location);

    virtual void make_overlays(VideoOverlaySet& items) const override{
        SelectionArrowDetector::make_overlays(items);
    }

    using VisualInferenceCallback::process_frame;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    // Contains the matching mode, location and grid coordinates after process_frame() succeeds.
    const std::optional<BoxSelection>& last_detected() const{ return m_last_detected; }

private:
    BoxSelectionMode m_mode;
    BoxSelectionLocation m_location;
    std::optional<BoxSelection> m_last_detected;
};

void add_tests_SelectionArrowDetector(UnitTestDatabase& database);

}
}
}
#endif
