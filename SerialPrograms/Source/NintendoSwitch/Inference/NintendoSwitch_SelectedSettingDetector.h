/*  Selected Setting Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_SelectedSettingDetector_H
#define PokemonAutomation_NintendoSwitch_SelectedSettingDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class SelectedSettingWatcher : public VisualInferenceCallback{
public:
    SelectedSettingWatcher(
        ImageFloatBox selected_box,
        ImageFloatBox not_selected_box1,
        ImageFloatBox not_selected_box2,
        ImageFloatBox representative_background = {0.60, 0.02, 0.35, 0.05}
    );
    virtual ~SelectedSettingWatcher();

    virtual void make_overlays(VideoOverlaySet& items) const override;

    bool is_white_theme(const ImageViewRGB32& screen);

    // return true if the area within the selected_box is highlighted, compared with the area within unselected_box
    // This compares the brightness of the selected_box with the unselected_box.
    // selected_box: the box where we expect the screen should be highlighted
    // not_selected_box 1 and 2: the boxes where we expect the screen should NOT be highlighted. These acts as the control, for comparison.
    // representative_background: box around an area that would represent the background. for determining whether this is white or dark mode.
    // the average sum of selected_box should be greater than the absolute difference of average sum between unselected_box 1 and 2.
    virtual bool process_frame(const ImageViewRGB32& screen, WallClock timestamp) override;


protected:
    ImageFloatBox m_selected_box;
    ImageFloatBox m_not_selected_box1;
    ImageFloatBox m_not_selected_box2;
    ImageFloatBox m_representative_background;
};


}
}
#endif

