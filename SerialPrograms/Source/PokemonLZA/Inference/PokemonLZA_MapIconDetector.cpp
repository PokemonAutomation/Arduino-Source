/*  Map Icon Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include "Common/Cpp/Exceptions.h"
//#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA_MapIconDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using ImageMatch::WaterfillTemplateMatcher;



class MapIconMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    MapIconMatcher(
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



const MapIconMatcher& MapIcon_PokemonCenter(){
    static MapIconMatcher ret(
        "PokemonLZA/MapIcons/PokemonCenter.png",
        Pokemon::STRING_POKEMON + " Center",
        Color(0xffc04030),
        Color(0xffff8f6f),
        100,
        100.0,
        {
            {0xffc04030, 0xffff8f6f},
            {0xffc05040, 0xffffaf8f},
        }
    );
    return ret;
}
const MapIconMatcher& MapIcon_Building(){
    static MapIconMatcher ret(
        "PokemonLZA/MapIcons/Building.png",
        "Building",
        Color(0xff000000),
        Color(0xff7f7f7f),
        100,
        100.0,
        {
            {0xff000000, 0xff7f7f7f},
        }
    );
    return ret;
}
const MapIconMatcher& MapIcon_BuildingFlyable(){
    static MapIconMatcher ret(
        "PokemonLZA/MapIcons/BuildingFlyable.png",
        "Building (Flyable)",
        Color(0xff000000),
        Color(0xff7f7f7f),
        100,
        100.0,
        {
            {0xff000000, 0xff7f7f7f},
        }
    );
    return ret;
}
const MapIconMatcher& MapIcon_CafeFlyable(){
    static MapIconMatcher ret(
        "PokemonLZA/MapIcons/CafeFlyable.png",
        "Cafe (Flyable)",
        Color(0xff000000),
        Color(0xff7f7f7f),
        100,
        100.0,
        {
            {0xff000000, 0xff7f7f7f},
        }
    );
    return ret;
}
const MapIconMatcher& MapIcon_WildZone(){
    static MapIconMatcher ret(
        "PokemonLZA/MapIcons/WildZone.png",
        "Wild Zone",
        Color(0xff000000),
        Color(0xff7f7f7f),
        50,
        50.0,
        {
            {0xff000000, 0xff7f7f7f},
        },
        0.646802
    );
    return ret;
}
const MapIconMatcher& MapIcon_WildZoneFlyable(){
    static MapIconMatcher ret(
        "PokemonLZA/MapIcons/WildZoneFlyable.png",
        "Wild Zone (Flyable)",
        Color(0xff000000),
        Color(0xff7f7f7f),
        50,
        50.0,
        {
            {0xff000000, 0xff7f7f7f},
        },
        0.680978
    );
    return ret;
}
const MapIconMatcher& MapIcon_BattleZone(){
    static MapIconMatcher ret(
        "PokemonLZA/MapIcons/BattleZone.png",
        "Battle Zone",
        Color(0xff000000),
        Color(0xff7f7f7f),
        100,
        50.0,
        {
            {0xff000000, 0xff7f7f7f},
        }
    );
    return ret;
}

const MapIconMatcher& get_map_icon_matcher(MapIconType icon){
    switch (icon){
    case MapIconType::PokemonCenter:
        return MapIcon_PokemonCenter();
    case MapIconType::Building:
        return MapIcon_Building();
    case MapIconType::BuildingFlyable:
        return MapIcon_BuildingFlyable();
    case MapIconType::CafeFlyable:
        return MapIcon_CafeFlyable();
    case MapIconType::WildZone:
        return MapIcon_WildZone();
    case MapIconType::WildZoneFlyable:
        return MapIcon_WildZoneFlyable();
    case MapIconType::BattleZone:
        return MapIcon_BattleZone();
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Unsupported Icon Type: " + std::to_string((int)icon)
        );
    }
}






MapIconDetector::MapIconDetector(
    Color color,
    MapIconType button_type,
    const ImageFloatBox& box,
    VideoOverlay* overlay
)
    : m_color(color)
    , m_matcher(get_map_icon_matcher(button_type))
    , m_box(box)
    , m_overlay(overlay)
{}
void MapIconDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool MapIconDetector::detect(const ImageViewRGB32& screen){

    double screen_rel_size = (screen.height() / 2160.0);
    double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    double min_area_3840p = 1000.0;
//    double rmsd_threshold = 120.0;
    size_t min_area = size_t(screen_rel_size_2 * min_area_3840p);

    m_last_detected.clear();

    match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        m_matcher,
        m_matcher.filters(),
        {min_area, SIZE_MAX},
        m_matcher.max_rmsd(),
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
//            cout << "width = " << object.width() << ", height = " << object.height() << endl;
            m_last_detected.emplace_back(
                Detection{
                    m_matcher.name(),
                    translate_to_parent(screen, m_box, object)
                }
            );
            return false;
        }
    );

    if (m_overlay){
        m_last_detected_box.clear();
        for (const Detection& detection : m_last_detected){
            m_last_detected_box.emplace_back(
                *m_overlay,
                COLOR_GREEN,
                detection.box,
                detection.name
            );
        }
    }

    return !m_last_detected_box.empty();
}






























}
}
}
