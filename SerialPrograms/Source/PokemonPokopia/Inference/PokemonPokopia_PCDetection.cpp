/*  PC Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonPokopia_PCDetection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{



class InfoIconMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    InfoIconMatcher(const char* path)
        : WaterfillTemplateMatcher(
            path,
            Color(210, 120, 0), Color(255, 190, 100), 700
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const InfoIconMatcher& matcher(){
        static InfoIconMatcher matcher("PokemonPokopia/InfoIcon.png");
        return matcher;
    }
};



InfoIconDetector::InfoIconDetector(
    Color color,
    VideoOverlay* overlay,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_arrow_box(box)
{}
void InfoIconDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_arrow_box);
}
bool InfoIconDetector::detect(const ImageViewRGB32& screen){
    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 700;
    double rmsd_threshold = 80;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffd26e00, 0xffffbe64} // RGB(210, 120, 0), RGB(255, 190, 100)
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_arrow_box),
        InfoIconMatcher::matcher(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(screen, m_arrow_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}

class StampMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    // image template matcher for stamps
    // - min_width: candidate image min width if video stream is 1080p
    // - min_height: candidate image min height if video stream is 1080p
    StampMatcher(Stamp stamp, size_t min_width, size_t min_height, double max_rmsd);
    static const StampMatcher& mew(){
        static StampMatcher matcher(Stamp::MEW, 40, 40, 70);
        return matcher;
    }

    virtual bool check_image(Resolution input_resolution, const ImageViewRGB32& image) const override{
        size_t min_width = m_min_width * input_resolution.width / 1920;
        size_t min_height = m_min_height * input_resolution.height / 1080;

        if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
            const double stddev_sum = image_stats(image).stddev.sum();
            std::cout << "???? ButtonMatcher::check_image() ???? min size " << min_width << " x " << min_height
                 << " got " << image.width() << " x " << image.height() << " stddev_sum: " << stddev_sum << std::endl;
        }
        return image.width() >= min_width && image.height() >= min_height;
    };

    size_t m_min_width;
    size_t m_min_height;
    double m_max_rmsd;
};


const char* template_path(Stamp stamp){
    switch (stamp){
    case Stamp::MEW:
        return "PokemonPokopia/MewStamp.png";
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, "No corresponding template path for Stamp"
        );
    }
}

// Using same filter for template and detect for now
std::pair<uint32_t, uint32_t> filter(Stamp stamp){
    switch (stamp){
    case Stamp::MEW:
        return {0xff5a50aa, 0xffd2beff}; // RGB(90, 80, 170), RGB(210, 190, 255)
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, "No corresponding filter for Stamp"
        );
    }
}

const StampMatcher& get_stamp_matcher(Stamp stamp){
    switch (stamp){
    case Stamp::MEW:
        return StampMatcher::mew();
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, "No corresponding StampMatcher for Stamp"
        );
    }
}

StampMatcher::StampMatcher(Stamp stamp, size_t min_width, size_t min_height, double max_rmsd)
    : WaterfillTemplateMatcher(template_path(stamp), Color(filter(stamp).first), Color(filter(stamp).second), 100)
    , m_min_width(min_width)
    , m_min_height(min_height)
    , m_max_rmsd(max_rmsd)
{}



StampDetector::StampDetector(
    Color color,
    Stamp stamp,
    const ImageFloatBox& box,
    VideoOverlay* overlay
)
    : m_stamp(stamp)
    , m_color(color)
    , m_matcher(get_stamp_matcher(stamp))
    , m_box(box)
    , m_overlay(overlay)
{

}
void StampDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool StampDetector::detect(const ImageViewRGB32& screen){

    double screen_rel_size = (screen.height() / 1080.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_1080p = 800;
    size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        m_matcher,
        {filter(m_stamp)},
        {min_area, SIZE_MAX},
        m_matcher.m_max_rmsd,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(screen, m_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}



}
}
}
