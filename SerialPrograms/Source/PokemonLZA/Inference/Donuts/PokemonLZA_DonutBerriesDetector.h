/*  Donut Berries Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DonutBerriesDetector_H
#define PokemonAutomation_PokemonLZA_DonutBerriesDetector_H

#include <array>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/ImageMatch/ImageMatchResult.h"
#include "CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonLZA{


// Detect the vertical pink bar on the left of the selected berry's row in the donut maker.
class DonutBerriesSelectionDetector : public StaticScreenDetector{
public:
    // menu_index: which row of the selection arrow should be on. Range: [0, 7]:
    // there are at most 8 rows on screen.
    DonutBerriesSelectionDetector(size_t menu_index);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_symbol_box;
};
class DonutBerriesSelectionWatcher : public DetectorToFinder<DonutBerriesSelectionDetector>{
public:
    DonutBerriesSelectionWatcher(size_t menu_index)
         : DetectorToFinder("DonutBerriesSelectionWatcher", std::chrono::milliseconds(250), menu_index)
    {}
};


// Call `ImageMatchResult match(image, alpha_spread)` to find which berry is the input image.
// The berry sprites and their slugs are defined in `DONUT_BERRIES_DATABASE()`
class DonutBerriesMatcher : public ImageMatch::CroppedImageDictionaryMatcher{
public:
    DonutBerriesMatcher(const std::vector<double>& min_euclidean_distance = {100, 150, 200});

private:
    virtual std::vector<ImageViewRGB32> get_crop_candidates(const ImageViewRGB32& image) const override;
    std::vector<double> m_min_euclidean_distance_squared;
};


class DonutBerriesOCR : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.5;
    static constexpr double MAX_LOG10P_SPREAD = 0.5;

public:
    static const DonutBerriesOCR& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;

private:
    DonutBerriesOCR();
};


class DonutBerriesReader{
public:
    static constexpr double MAX_ALPHA = 180;
    static constexpr double ALPHA_SPREAD = 10;
    static constexpr size_t BERRY_PAGE_LINES = 8;

public:
    DonutBerriesReader(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;

    std::array<ImageFloatBox, 8> berry_list_boxes(ImageFloatBox initial_box);
    ImageMatch::ImageMatchResult read_berry_page_with_icon_matcher(const ImageViewRGB32& screen, size_t index) const;
    ImageMatch::ImageMatchResult read_with_icon_matcher(const ImageViewRGB32& screen, const ImageFloatBox icon_box) const;

    OCR::StringMatchResult read_berry_page_with_ocr(
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

    Color m_color;
    std::array<ImageFloatBox, BERRY_PAGE_LINES> m_box_berry_text;
    std::array<ImageFloatBox, BERRY_PAGE_LINES> m_box_berry_icon;
};

}
}
}
#endif
