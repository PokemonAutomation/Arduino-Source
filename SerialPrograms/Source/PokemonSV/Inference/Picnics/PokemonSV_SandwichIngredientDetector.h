/*  Sandwich Ingredient Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_SandwichIngredientDetector_H
#define PokemonAutomation_PokemonSV_SandwichIngredientDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

#include <array>

namespace PokemonAutomation{
    class ConsoleHandle;
    class BotBaseContext;
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{

//  Detect the selection arrow in the sandwich ingredient list (the list of fillings, condiments and picks).
class SandwichIngredientArrowDetector : public StaticScreenDetector{
public:
    // menu_index: which row of the selection arrow should be on. Range: [0, 9]:
    // there are at most 10 rows on screen.
    SandwichIngredientArrowDetector(size_t menu_index, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    GradientArrowDetector m_arrow;
};

class SandwichIngredientArrowWatcher : public DetectorToFinder<SandwichIngredientArrowDetector>{
public:
    SandwichIngredientArrowWatcher(size_t menu_index, Color color = COLOR_RED)
         : DetectorToFinder("SandwichIngredientArrowWatcher", std::chrono::milliseconds(250), menu_index, color)
    {}
};



enum class SandwichIngredientType{
    FILLING,
    CONDIMENT,
};

//  Detect one determined sandwich ingredient, displayed in the lower right corner, inside a highlighted yellow box
//  The detector detects this yellow box.
class DeterminedSandwichIngredientDetector : public StaticScreenDetector{
public:
    // index: the `index`-th ingredient to detect.
    // For single player, there are at most 6 fillings, so if `ingredient_type` == FILLING, `index` range is [0, 5].
    // For single player, there are at most 4 condiments, so if `ingredient_type` == CONDIMENT, `index` range is [0, 3].
    DeterminedSandwichIngredientDetector(SandwichIngredientType ingredient_type, size_t index, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    std::array<ImageFloatBox, 4> m_edges;
};

class DeterminedSandwichIngredientWatcher : public DetectorToFinder<DeterminedSandwichIngredientDetector>{
public:
    DeterminedSandwichIngredientWatcher(SandwichIngredientType ingredient_type, size_t index, Color color = COLOR_RED)
         : DetectorToFinder("DeterminedSandwichIngredientWatcher", std::chrono::milliseconds(250), ingredient_type, index, color)
    {}
};

// Detect the white condiments page at upper left corner of the screen, during custom sandwich mode.
class SandwichCondimentsPageDetector : public StaticScreenDetector{
public:
    SandwichCondimentsPageDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_box;
};
class SandwichCondimentsPageWatcher : public DetectorToFinder<SandwichCondimentsPageDetector>{
public:
    SandwichCondimentsPageWatcher(Color color = COLOR_RED)
         : DetectorToFinder("SandwichCondimentsPageWatcher", std::chrono::milliseconds(250), color)
    {}
};

// Detect the white picks page at upper left corner of the screen, during custom sandwich mode.
class SandwichPicksPageDetector : public StaticScreenDetector{
public:
    SandwichPicksPageDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_box;
};
class SandwichPicksPageWatcher : public DetectorToFinder<SandwichPicksPageDetector>{
public:
    SandwichPicksPageWatcher(Color color = COLOR_RED)
         : DetectorToFinder("SandwichPicksPageWatcher", std::chrono::milliseconds(250), color)
    {}
};



}
}
}
#endif
