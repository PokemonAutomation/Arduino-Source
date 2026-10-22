/*  Box Cell Occupied Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BoxCellOccupiedDetector_H
#define PokemonAutomation_PokemonFRLG_BoxCellOccupiedDetector_H

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

// Reads the current selection's information panel, not every cell in the grid.
class BoxCellOccupiedDetector : public StaticScreenDetector{
public:
    BoxCellOccupiedDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_box;
};

void add_tests_BoxCellOccupiedDetector(UnitTestDatabase& database);

}
}
}
#endif
