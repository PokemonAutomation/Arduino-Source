/*  Travel Locations
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <QString>
#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA_TravelLocations.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


TravelLocation::TravelLocation(
    const char* p_label,
    MapRegion p_region,
    uint8_t p_warp_slot, uint8_t p_warp_sub_slot,
    std::function<void(ConsoleHandle& console, BotBaseContext& context)>&& p_post_arrival_maneuver
)
    : label(p_label)
    , region(p_region)
    , warp_slot(p_warp_slot)
    , warp_sub_slot(p_warp_sub_slot)
    , post_arrival_maneuver(std::move(p_post_arrival_maneuver))
{}


const TravelLocations& TravelLocations::instance(){
    static const TravelLocations locations;
    return locations;
}
const TravelLocation* TravelLocations::get_from_name(const std::string& name) const{
    auto iter = m_map.find(name);
    if (iter != m_map.end()){
        return iter->second;
    }
    return nullptr;
}
std::vector<QString> TravelLocations::all_location_names() const{
    std::vector<QString> ret;
    for (const TravelLocation* item : m_list){
        ret.emplace_back(item->label);
    }
    return ret;
}


void TravelLocations::add_location(const TravelLocation& location){
    if (!m_map.emplace(location.label, &location).second){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            std::string("Duplicate TravelLocation name: ") + location.label
        );
    }
    m_list.emplace_back(&location);
}
TravelLocations::TravelLocations()
    : Fieldlands_Fieldlands(
        "Obsidian Fieldlands - Fieldlands Camp",
        MapRegion::FIELDLANDS, 0, 0, nullptr
    )
    , Fieldlands_Heights(
        "Obsidian Fieldlands - Heights Camp",
        MapRegion::FIELDLANDS, 1, 0, nullptr
    )
    , Fieldlands_Arena(
        "Obsidian Fieldlands - Grandtree Arena",
        MapRegion::FIELDLANDS, 0, 2, nullptr
    )

    , Mirelands_Mirelands(
        "Crimson Mirelands - Mirelands Camp",
        MapRegion::MIRELANDS, 0, 0, nullptr
    )
    , Mirelands_Bogbound(
        "Crimson Mirelands - Bogbound Camp",
        MapRegion::MIRELANDS, 1, 0, nullptr
    )
    , Mirelands_DiamondSettlement(
        "Crimson Mirelands - Diamond Settlement",
        MapRegion::MIRELANDS, 0, 2, nullptr
    )
    , Mirelands_Arena(
        "Crimson Mirelands - Brava Arena",
        MapRegion::MIRELANDS, 1, 3, nullptr
    )

    , Coastlands_Beachside(
        "Cobalt Coastlands - Beachside Camp",
        MapRegion::COASTLANDS, 0, 0, nullptr
    )
    , Coastlands_Coastlands(
        "Cobalt Coastlands - Coastlands Camp",
        MapRegion::COASTLANDS, 1, 0, nullptr
    )
    , Coastlands_Arena(
        "Cobalt Coastlands - Molten Arena",
        MapRegion::COASTLANDS, 0, 2, nullptr
    )
    , Coastlands_Arena_NW(
        "Cobalt Coastlands - Molten Arena (NW of Volcano)",
        MapRegion::COASTLANDS, 0, 2, [](ConsoleHandle& console, BotBaseContext& context){
            change_mount(console, context, MountState::BRAVIARY_ON);
            pbf_move_left_joystick(context, 160, 0, 160, 0);
            pbf_mash_button(context, BUTTON_B, 4 * TICKS_PER_SECOND);
        }
    )

    , Highlands_Highlands(
        "Coronet Highlands - Highlands Camp",
        MapRegion::HIGHLANDS, 0, 0, nullptr
    )
    , Highlands_Mountain(
        "Coronet Highlands - Mountain Camp",
        MapRegion::HIGHLANDS, 1, 0, nullptr
    )
    , Highlands_Summit(
        "Coronet Highlands - Summit Camp",
        MapRegion::HIGHLANDS, 2, 0, nullptr
    )
    , Highlands_Arena(
        "Coronet Highlands - Moonview Arena",
        MapRegion::HIGHLANDS, 0, 3, nullptr
    )

    , Icelands_Snowfields(
        "Alabaster Icelands - Snowfields Camp",
        MapRegion::ICELANDS, 0, 0, nullptr
    )
    , Icelands_Icepeak(
        "Alabaster Icelands - Icepeak Camp",
        MapRegion::ICELANDS, 1, 0, nullptr
    )
    , Icelands_PearlSettlement(
        "Alabaster Icelands - Pearl Settlement",
        MapRegion::ICELANDS, 0, 2, nullptr
    )
    , Icelands_Arena(
        "Alabaster Icelands - Icepeak Arena",
        MapRegion::ICELANDS, 0, 3, nullptr
    )
{
    add_location(Fieldlands_Fieldlands);
    add_location(Fieldlands_Heights);
    add_location(Fieldlands_Arena);

    add_location(Mirelands_Mirelands);
    add_location(Mirelands_Bogbound);
    add_location(Mirelands_DiamondSettlement);
    add_location(Mirelands_Arena);

    add_location(Coastlands_Beachside);
    add_location(Coastlands_Coastlands);
    add_location(Coastlands_Arena);
    add_location(Coastlands_Arena_NW);

    add_location(Highlands_Highlands);
    add_location(Highlands_Mountain);
    add_location(Highlands_Summit);
    add_location(Highlands_Arena);

    add_location(Icelands_Snowfields);
    add_location(Icelands_Icepeak);
    add_location(Icelands_PearlSettlement);
    add_location(Icelands_Arena);
}









}
}
}
