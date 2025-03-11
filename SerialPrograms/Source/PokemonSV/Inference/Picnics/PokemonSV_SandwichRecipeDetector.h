/*  Sandwich Recipe Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_SandwichRecipeNumberDetector_H
#define PokemonAutomation_PokemonSV_SandwichRecipeNumberDetector_H

#include "Common/Cpp/Color.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{

// Detect the sandwich recipe IDs
class SandwichRecipeNumberDetector{
public:
    SandwichRecipeNumberDetector(Logger& logger, Color color = COLOR_YELLOW);

    void make_overlays(VideoOverlaySet& items) const;

    // Detect the IDs of the displayed sandwich recipes.
    // The order is from top to bottom, left to right:
    // --------------------------------------
    // recipe_IDs[0]  |   recipe_IDs[1]
    // recipe_IDs[2]  |   recipe_IDs[3]
    // recipe_IDs[4]  |   recipe_IDs[5]
    // --------------------------------------
    // recipe ID range: [1, 151]
    // If there is no recipe at one cell, its value is 0.
    // If a recipe has not enough ingredient to make (displayed as semi-transparent), it value is 0.
    void detect_recipes(const ImageViewRGB32& screen, size_t recipe_IDs[6]) const;

private:
    Logger& m_logger;
    Color m_color;
    ImageFloatBox m_id_boxes[6];
};

// Find which recipe cell is selected by the gradient arrow when choosing a sandwich recipe.
// Run `process_frame()` until it returns true, meaning arrow detected, then call `selected_recipe_cell()`
// to get the detected cell.
class SandwichRecipeSelectionWatcher : public VisualInferenceCallback{
public: 
    SandwichRecipeSelectionWatcher(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const VideoSnapshot& frame) override;

    // Whether it can detect the recipe selection on this frame.
    // Unlike `process_frame()` which relies on consecutive frames to give robust result,
    // this function uses only one frame.
    bool detect(const ImageViewRGB32& frame);

    // Return which recipe cell is currented selected.
    // For cell order, see `SandwichRecipeNumberDetector`.
    // If no cell selection detected, return -1.
    int selected_recipe_cell() const { return m_selected_recipe; }

private:
    FixedLimitVector<GradientArrowWatcher> m_arrow_watchers;
    int m_selected_recipe = -1;
};




}
}
}
#endif
