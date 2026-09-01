/*  Start Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_StartMenuDetector_H
#define PokemonAutomation_PokemonRSE_StartMenuDetector_H

#include <chrono>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonRSE/Inference/PokemonRSE_SelectionDetector.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonRSE{

enum class StartMenuState {
    PRE_POKEMON = 0,
    PRE_POKEDEX = 1,
    PRE_POKENAV = 2,
    FULL = 3,
    SAFARI = 4,
};

enum class StartMenuSlot {
    POKEDEX = 0,
    POKEMON = 1,
    BAG = 2,
    POKENAV = 3,
    TRAINER = 4,
    SAVE = 5,
    OPTION = 6,
    EXIT = 7,
    RETIRE = 8,
};

// Detect the full start menu by looking for empty white sections on the top and bottom
// Works for all RSE, all languages.
// No arrow detection, RS japan and all emerald languages have an arrow,
// but RS for all non-japan languages use a red box for selection.
class StartMenuDetector : public StaticScreenDetector{
public:
    StartMenuDetector(Color color, StartMenuState state = StartMenuState::FULL);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_menu_top_box;
    ImageFloatBox m_menu_bottom_box;
};
class StartMenuWatcher : public DetectorToFinder<StartMenuDetector>{
public:
    StartMenuWatcher(Color color = COLOR_RED, StartMenuState state = StartMenuState::FULL)
        : DetectorToFinder("StartMenuWatcher", std::chrono::milliseconds(250), color, state)
    {}
};

// Detect whether a specified slot is selected
class StartMenuSlotDetector : public SelectionSlotDetector {
public:
    StartMenuSlotDetector(Color color, StartMenuSlot slot, StartMenuState state = StartMenuState::FULL);
};
class StartMenuSlotWatcher : public DetectorToFinder<StartMenuSlotDetector> {
public:
    StartMenuSlotWatcher(Color color, StartMenuSlot slot, StartMenuState state = StartMenuState::FULL)
        : DetectorToFinder("StartMenuSlotWatcher", std::chrono::milliseconds(250), color, slot, state)
    {}
};


void add_tests_StartMenuDetector(UnitTestDatabase& database);
void add_tests_StartMenuSlotDetector(UnitTestDatabase& database);

}
}
}

#endif
