/*  Waterfill Template Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_WaterfillTemplateMatcher_H
#define PokemonAutomation_CommonFramework_WaterfillTemplateMatcher_H

#include <memory>
#include "Common/Compiler.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"

namespace PokemonAutomation{
namespace ImageMatch{


class WaterfillTemplateMatcher{
protected:
    using WaterfillObject = Kernels::Waterfill::WaterfillObject;

public:
    WaterfillTemplateMatcher(WaterfillTemplateMatcher&&) = default;
    virtual ~WaterfillTemplateMatcher() = default;


public:
    // Load a template image from disk, min_color and max_color denote the
    // color range of the object displayed in the image file while min_area
    // is the minimum number of pixels required for an object.
    WaterfillTemplateMatcher(
        const char* path,
        Color min_color, Color max_color,
        size_t min_area
    );

    //  Compute RMSD of current object against the template as-is.
    double rmsd(const ConstImageRef& object) const;

    virtual double rmsd_precropped(const ConstImageRef& cropped_image, const WaterfillObject& object) const;
    virtual double rmsd_original(const ConstImageRef& original_image, const WaterfillObject& object) const;

protected:
    virtual bool check_image(const ConstImageRef& image) const{ return true; };
    bool check_aspect_ratio(size_t candidate_width, size_t candidate_height) const;
    bool check_area_ratio(double candidate_area_ratio) const;

protected:
    QImage m_object;

    double m_aspect_ratio_lower = 0.80;
    double m_aspect_ratio_upper = 1.25;
    double m_area_ratio_lower = 0.80;
    double m_area_ratio_upper = 1.25;

    std::unique_ptr<ExactImageMatcher> m_matcher;
    double m_area_ratio;
};



}
}
#endif
