/*  Origin Mark Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonHome_OriginMarkReader.h"
#include <map>
#include "CommonTools/Images/WaterfillUtilities.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include <sstream>
#include <vector>
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonFramework/Globals.h"
#include "CommonTools/DetectedBoxes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

using ImageMatch::WaterfillTemplateMatcher;
using namespace Kernels;
using namespace Kernels::Waterfill;

class OriginMarkSprite {
public:
    OriginMarkSprite(const std::string& slug, const std::string& resource_location)
        : m_slug(slug)
    {
        ImageRGB32 sprite(RESOURCE_PATH() + resource_location);

        sprite = filter_rgb32_range(
            sprite,
            0xff000000, 0xffffffff,
            Color(0), false
        );

        PackedBinaryMatrix matrix = compress_rgb32_to_binary_min(sprite, 93, 100, 83);
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 10);

        WaterfillObject object;
        for (const WaterfillObject& item : objects){
            object.merge_assume_no_overlap(item);
        }

        m_aspect_ratio = object.aspect_ratio();
        m_matcher = std::make_unique<ImageMatch::WeightedExactImageMatcher>(
            sprite.sub_image(object.min_x, object.min_y, object.width(), object.height()).copy(),
            ImageMatch::WeightedExactImageMatcher::InverseStddevWeight{ 1, 64 }
        );
    }

    const std::string& slug() const { return m_slug; }
    double aspect_ratio() const { return m_aspect_ratio; }
    const ImageMatch::WeightedExactImageMatcher& matcher() const { return *m_matcher; }

private:
    std::string m_slug;
    double m_aspect_ratio;
    std::unique_ptr<ImageMatch::WeightedExactImageMatcher> m_matcher;
};

struct OriginMarkSpriteDatabase {
    std::map<OriginMark, OriginMarkSprite> m_mark_map;

    static OriginMarkSpriteDatabase& instance(){
        static OriginMarkSpriteDatabase data;
        return data;
    }

    OriginMarkSpriteDatabase(){
        for (const auto& item : Pokemon::ORIGIN_MARK_SLUGS()){
            if (item.first == OriginMark::NONE || item.first == OriginMark::GAMEBOY){ //TODO: Need to find someone with the gameboy sprite
                continue;
            }
            m_mark_map.emplace(item.first, OriginMarkSprite(item.second, "PokemonHome/OriginMarks/" + item.second + ".png"));
        }
    }
};

class MarkIconMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MarkIconMatcher(
        const char* path,
        std::string name,
        Color min_color, Color max_color,
        size_t min_area,
        double max_rmsd,
        std::vector<std::pair<uint32_t, uint32_t>> filters,
        double area_ratio_override = 0
    )
        : WaterfillTemplateMatcher(path, min_color, max_color, min_area)
        , m_name(std::move(name))
        , m_max_rmsd(max_rmsd)
        , m_filters(std::move(filters))
    {
        if (area_ratio_override != 0){
            m_area_ratio = area_ratio_override;
        }
    }

    const std::string& name() const{
        return m_name;
    }
    double max_rmsd() const{
        return m_max_rmsd;
    }
    const std::vector<std::pair<uint32_t, uint32_t>>& filters() const{
        return m_filters;
    }

private:
    std::string m_name;
    double m_max_rmsd;
    std::vector<std::pair<uint32_t, uint32_t>> m_filters;
};

const MarkIconMatcher& MarkIcon_Kalos(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/kalos.png",
        "kalos",
        Color(0xff61645c), Color(0xff646664),
        10,
        40.0,
        {
            {0xff61645c, 0xff646664},
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Alola(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/alola.png",
        "alola",
        Color(0xff5f6456), Color(0xff6d706b),
        10,
        35.0,
        {
            {0xff5f6456, 0xff6d706b},
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Go(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/go.png",
        "go",
        Color(0xff5c644e), Color(0xff798073),
        10,
        45.0,
        {
            {0xff5c644e, 0xff798073},
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Lgpe(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/lgpe.png",
        "lgpe",
        Color(0xff616459), Color(0xff676d64),
        10,
        20.0,
        {
            {0xff616459, 0xff676d64},
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Galar(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/galar.png",
        "galar",
        Color(0xff5e6453), Color(0xff757870),
        10,
        45.0,
        {
            {0xff5e6453, 0xff757870},
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Bdsp(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/bdsp.png",
        "bdsp",
        Color(0xff5f6457), Color(0xff767972),
        10,
        45.0,
        {
            {0xff5f6457, 0xff767972},
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_La(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/la.png",
        "la",
        Color(0xff606459), Color(0xff686a65),
        10,
        30.0,
        {
            {0xff606459, 0xff686a65},
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Sv(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/sv.png",
        "sv",
        Color(0xff5d6450), Color(0xff747771),
        10,
        50.0,
        {
            {0xff5d6450, 0xff747771},
        }
    );
    return ret;
}

const MarkIconMatcher& MarkIcon_Lza(){
    static MarkIconMatcher ret(
        "PokemonHome/OriginMarks/lza.png",
        "lza",
        Color(0xff61645b), Color(0xff646764),
        10,
        20.0,
        {
            {0xff61645b, 0xff646764},
        }
    );
    return ret;
}

//TODO: GAMEBOY

const std::vector<const MarkIconMatcher*>& ALL_ORIGIN_MARK_MATCHERS(){
    static const std::vector<const MarkIconMatcher*> matchers = {
        &MarkIcon_Kalos(),
        &MarkIcon_Alola(),
        &MarkIcon_Go(),
        &MarkIcon_Lgpe(),
        &MarkIcon_Galar(),
        &MarkIcon_Bdsp(),
        &MarkIcon_La(),
        &MarkIcon_Sv(),
        &MarkIcon_Lza(),
    };
    return matchers;
}

const MarkIconMatcher& get_mark_icon_matcher(OriginMark mark){
    switch (mark){
    case OriginMark::KALOS:
        return MarkIcon_Kalos();
    case OriginMark::ALOLA:
        return MarkIcon_Alola();
    case OriginMark::GO:
        return MarkIcon_Go();
    case OriginMark::LGPE:
        return MarkIcon_Lgpe();
    case OriginMark::GALAR:
        return MarkIcon_Galar();
    case OriginMark::BDSP:
        return MarkIcon_Bdsp();
    case OriginMark::LA:
        return MarkIcon_La();
    case OriginMark::SV:
        return MarkIcon_Sv();
    case OriginMark::LZA:
        return MarkIcon_Lza();
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid origin mark: " + std::to_string((int)mark));
    }
}

OriginMark origin_mark_from_slug(const std::string& slug){
    for (const auto& item : Pokemon::ORIGIN_MARK_SLUGS()){
        if (item.second == slug){
            return item.first;
        }
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid origin mark slug: " + slug);
}

OriginMark OriginMarkReader::read_mark(
    const ImageViewRGB32& original_screen,
    const ImageFloatBox& box
){
    const double screen_rel_size = (original_screen.height() / 1080.0);
    const double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    const double min_area_1080p = 400;
    const size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    ImageViewRGB32 image = extract_box_reference(original_screen, box);
    const ImageStats roi_stats = image_stats(image);

    m_last_detected.clear();

    std::multimap<double, std::pair<OriginMark, ImagePixelBox>> candidates;

    for (const MarkIconMatcher* matcher : ALL_ORIGIN_MARK_MATCHERS()){
        match_template_by_waterfill(
            original_screen.size(),
            image,
            *matcher,
            matcher->filters(),
            { min_area, SIZE_MAX },
            matcher->max_rmsd(),
            [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
                
                m_last_detected.emplace_back(
                    DetectedBox{
                        matcher->name(),
                        translate_to_parent(original_screen, box, object)
                    }
                );
                return true;
            }
        );

        if (!m_last_detected.empty()){
            return origin_mark_from_slug(m_last_detected[0].name);
        }
    }

    return OriginMark::NONE;
}


}
}
}
