/*  Sub-Object Template Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_SubObjectTemplateMatcher_H
#define PokemonAutomation_CommonFramework_SubObjectTemplateMatcher_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
    class WaterfillObject;
}
}
namespace ImageMatch{


class SubObjectTemplateMatcher{
public:
    virtual ~SubObjectTemplateMatcher() = default;
    SubObjectTemplateMatcher(const char* path, double max_rmsd);

    ImagePixelBox object_from_subobject(const ImagePixelBox& subobject_in_image) const;

    double rmsd(
        ImagePixelBox& object_box,
        const QImage& image, const ImagePixelBox& subobject_in_image
    ) const;

    virtual bool matches(
        ImagePixelBox& object_box,
        const QImage& image, const ImagePixelBox& subobject_in_image
    ) const;


protected:
    void set_subobject(const ImagePixelBox& subobject_in_object);
    bool check_aspect_ratio(size_t width, size_t height) const;

protected:
    QImage m_object;
    double m_max_rmsd;
    double m_aspect_ratio_min;
    double m_aspect_ratio_max;

    ImageMatch::ExactImageMatcher m_matcher;
    ImagePixelBox m_subobject_in_object_p;
    ImageFloatBox m_subobject_in_object_f;
};



}
}
#endif
