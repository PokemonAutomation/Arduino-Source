/*  Sandwich Ingredient Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_SandwichIngredientDetector_H
#define PokemonAutomation_PokemonSV_SandwichIngredientDetector_H

#include <array>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/ImageMatch/ImageMatchResult.h"
#include "CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
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
    virtual bool detect(const ImageViewRGB32& screen) override;

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
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    std::array<ImageFloatBox, 4> m_edges;
};

class DeterminedSandwichIngredientWatcher : public DetectorToFinder<DeterminedSandwichIngredientDetector>{
public:
    DeterminedSandwichIngredientWatcher(SandwichIngredientType ingredient_type, size_t index, Color color = COLOR_RED)
         : DetectorToFinder("DeterminedSandwichIngredientWatcher", std::chrono::milliseconds(100), ingredient_type, index, color)
    {}
};

// Detect the white condiments page at upper left corner of the screen, during custom sandwich mode.
class SandwichCondimentsPageDetector : public StaticScreenDetector{
public:
    SandwichCondimentsPageDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

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
    virtual bool detect(const ImageViewRGB32& screen) override;

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



class SandwichFillingMatcher : public ImageMatch::CroppedImageDictionaryMatcher{
public:
    SandwichFillingMatcher(const std::vector<double>& min_euclidean_distance = {100, 150, 200});

private:
    virtual std::vector<ImageViewRGB32> get_crop_candidates(const ImageViewRGB32& image) const override;
    std::vector<double> m_min_euclidean_distance_squared;
};

class SandwichCondimentMatcher : public ImageMatch::CroppedImageDictionaryMatcher{
public:
    SandwichCondimentMatcher(const std::vector<double>& min_euclidean_distance = {100, 150, 200});

private:
    virtual std::vector<ImageViewRGB32> get_crop_candidates(const ImageViewRGB32& image) const override;
    std::vector<double> m_min_euclidean_distance_squared;
};

class SandwichFillingOCR : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -2.0;
    static constexpr double MAX_LOG10P_SPREAD = 0.5;

public:
    static const SandwichFillingOCR& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;

private:
    SandwichFillingOCR();
};
class SandwichCondimentOCR : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.5;
    static constexpr double MAX_LOG10P_SPREAD = 0.5;

public:
    static const SandwichCondimentOCR& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;

private:
    SandwichCondimentOCR();
};

class SandwichIngredientReader{
public:
    static constexpr double MAX_ALPHA = 180;
    static constexpr double ALPHA_SPREAD = 10;
    static constexpr size_t INGREDIENT_PAGE_LINES = 10;

public:
    SandwichIngredientReader(SandwichIngredientType ingredient_type, Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;

    std::array<ImageFloatBox, 6> confirmed_ingredient_boxes(SandwichIngredientType type);

    std::array<ImageFloatBox, 10> ingredient_list_boxes(ImageFloatBox initial_box);

    ImageMatch::ImageMatchResult read_ingredient_page_with_icon_matcher(const ImageViewRGB32& screen, size_t index) const;

    ImageMatch::ImageMatchResult read_confirmed_list_with_icon_matcher(const ImageViewRGB32& screen, size_t index) const;

    // The icon matcher only works on the selected item, because we want to remove the yellow / orange background
    ImageMatch::ImageMatchResult read_with_icon_matcher(const ImageViewRGB32& screen, const ImageFloatBox icon_box) const;

    OCR::StringMatchResult read_ingredient_page_with_ocr(
        const ImageViewRGB32& screen, 
        Logger& logger, 
        Language language, 
        size_t index
    ) const;

    // The OCR works on any ingredient, selected or not
    OCR::StringMatchResult read_with_ocr(
        const ImageViewRGB32& screen, 
        Logger& logger, 
        Language language, 
        const ImageFloatBox icon_box
    ) const;

// private:
    Color m_color;
    SandwichIngredientType m_ingredient_type;
    std::array<ImageFloatBox, INGREDIENT_PAGE_LINES> m_box_ingred_text;
    std::array<ImageFloatBox, INGREDIENT_PAGE_LINES> m_box_ingred_icon;
    std::array<ImageFloatBox, 6> m_box_confirmed;
};

}
}
}
#endif
