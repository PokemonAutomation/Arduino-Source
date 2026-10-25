/*  Box Selection Mode Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BoxSelectionModeDetector_H
#define PokemonAutomation_PokemonFRLG_BoxSelectionModeDetector_H

#include <optional>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

enum class BoxSelectionMode{
    Normal,
    Relocate,
};

enum class BoxSelectionLocation{
    Grid,
    BoxTile,
    OpenPartyButton,
    CloseBoxButton,
};

struct BoxSelection{
    BoxSelectionMode mode;
    BoxSelectionLocation location;
    bool closed = false;
    // Zero-based coordinates in the 5-row, 6-column grid. Only valid for Grid.
    size_t row = 0;
    size_t column = 0;
};

class BoxSelectionModeDetector{
public:
    BoxSelectionModeDetector(Color color = COLOR_RED) : m_color(color) {}

    void make_overlays(VideoOverlaySet& items) const;
    // Returns no value when no selection hand can be matched.
    std::optional<BoxSelection> detect(const ImageViewRGB32& screen) const;

private:
    Color m_color;
};

class BoxSelectionModeWatcher : public BoxSelectionModeDetector, public VisualInferenceCallback{
public:
    BoxSelectionModeWatcher(
        Color color,
        BoxSelectionMode mode,
        BoxSelectionLocation location,
        bool closed = false
    );

    virtual void make_overlays(VideoOverlaySet& items) const override{
        BoxSelectionModeDetector::make_overlays(items);
    }

    using VisualInferenceCallback::process_frame;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    // Contains the matching hand and cursor coordinates after process_frame() succeeds.
    const std::optional<BoxSelection>& last_detected() const{ return m_last_detected; }

private:
    BoxSelectionMode m_mode;
    BoxSelectionLocation m_location;
    bool m_closed;
    std::optional<BoxSelection> m_last_detected;
};

void add_tests_BoxSelectionModeDetector(UnitTestDatabase& database);

}
}
}
#endif
