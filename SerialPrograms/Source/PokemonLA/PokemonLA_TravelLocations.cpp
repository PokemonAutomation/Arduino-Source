/*  Travel Locations
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA_TravelLocations.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


TravelLocation::TravelLocation(
    const char* p_slug, const char* p_display,
    MapRegion p_region,
    uint8_t p_warp_slot, uint8_t p_warp_sub_slot,
    std::function<void(VideoStream& stream, ProControllerContext& context)>&& p_post_arrival_maneuver,
    bool p_reverse_sub_menu_direction
)
    : slug(p_slug)
    , display(p_display)
    , region(p_region)
    , warp_slot(p_warp_slot)
    , warp_sub_slot(p_warp_sub_slot)
    , reverse_sub_menu_direction(p_reverse_sub_menu_direction)
    , post_arrival_maneuver(std::move(p_post_arrival_maneuver))
{}


const TravelLocations& TravelLocations::instance(){
    static const TravelLocations locations;
    return locations;
}


void TravelLocations::add_location(const TravelLocation& location, bool inside_village){
    if (!m_map.emplace(location.display, &location).second){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            std::string("Duplicate TravelLocation name: ") + location.display
        );
    }
    m_list.emplace_back(&location);
    m_database_include_village.add(m_list.size() - 1, location.slug, location.display, true);
    
    if (!inside_village){
        m_database_outside_village.add(m_list.size() - 1, location.slug, location.display, true);    
    }
    
}
TravelLocations::TravelLocations()
    : Fieldlands_Fieldlands(
        "fieldlands-fieldlands",
        "Obsidian Fieldlands - Fieldlands Camp",
        MapRegion::FIELDLANDS, 0, 0, nullptr
    )
    , Fieldlands_Heights(
        "fieldlands-heights",
        "Obsidian Fieldlands - Heights Camp",
        MapRegion::FIELDLANDS, 1, 0, nullptr
    )
    , Fieldlands_Arena(
        "fieldlands-arena",
        "Obsidian Fieldlands - Grandtree Arena",
        MapRegion::FIELDLANDS, 0, 2, nullptr
    )

    , Mirelands_Mirelands(
        "mirelands-mirelands",
        "Crimson Mirelands - Mirelands Camp",
        MapRegion::MIRELANDS, 0, 0, nullptr
    )
    , Mirelands_Bogbound(
        "mirelands-bogboung",
        "Crimson Mirelands - Bogbound Camp",
        MapRegion::MIRELANDS, 1, 0, nullptr
    )
    , Mirelands_DiamondSettlement(
        "mirelands-settlement",
        "Crimson Mirelands - Diamond Settlement",
        MapRegion::MIRELANDS, 0, 2, nullptr
    )
    , Mirelands_Arena(
        "mirelands-arena",
        "Crimson Mirelands - Brava Arena",
        MapRegion::MIRELANDS, 0, 2, nullptr, true
    )

    , Coastlands_Beachside(
        "coastlands-beachside",
        "Cobalt Coastlands - Beachside Camp",
        MapRegion::COASTLANDS, 0, 0, nullptr
    )
    , Coastlands_Coastlands(
        "coastlands-coastlands",
        "Cobalt Coastlands - Coastlands Camp",
        MapRegion::COASTLANDS, 1, 0, nullptr
    )
    , Coastlands_Arena(
        "coastlands-arena",
        "Cobalt Coastlands - Molten Arena",
        MapRegion::COASTLANDS, 0, 2, nullptr
    )
    , Coastlands_Arena_NW(
        "coastlands-arena-nw",
        "Cobalt Coastlands - Molten Arena (NW of Volcano)",
        MapRegion::COASTLANDS, 0, 2, [](VideoStream& stream, ProControllerContext& context){
            change_mount(stream, context, MountState::BRAVIARY_ON);
            pbf_move_left_joystick(context, 160, 0, 160, 0);
            pbf_mash_button(context, BUTTON_B, 4 * TICKS_PER_SECOND);
        }
    )

    , Highlands_Highlands(
        "highlands-highlands",
        "Coronet Highlands - Highlands Camp",
        MapRegion::HIGHLANDS, 0, 0, nullptr
    )
    , Highlands_Mountain(
        "highlands-mountain",
        "Coronet Highlands - Mountain Camp",
        MapRegion::HIGHLANDS, 1, 0, nullptr
    )
    , Highlands_Summit(
        "highlands-summit",
        "Coronet Highlands - Summit Camp",
        MapRegion::HIGHLANDS, 2, 0, nullptr
    )
    , Highlands_Arena(
        "highlands-arena",
        "Coronet Highlands - Moonview Arena",
        MapRegion::HIGHLANDS, 0, 2, nullptr, true
    )

    , Icelands_Snowfields(
        "icelands-icelands",
        "Alabaster Icelands - Snowfields Camp",
        MapRegion::ICELANDS, 0, 0, nullptr
    )
    , Icelands_Icepeak(
        "icelands-icepeak",
        "Alabaster Icelands - Icepeak Camp",
        MapRegion::ICELANDS, 1, 0, nullptr
    )
    , Icelands_PearlSettlement(
        "icelands-settlement",
        "Alabaster Icelands - Pearl Settlement",
        MapRegion::ICELANDS, 0, 2, nullptr
    )
    , Icelands_PearlSettlement_SW(
        "icelands-settlement-sw",
        "Alabaster Icelands - Pearl Settlement (SW of landing spot)",
        MapRegion::ICELANDS, 0, 2, [](VideoStream& stream, ProControllerContext& context){
            change_mount(stream, context, MountState::BRAVIARY_ON);
            pbf_move_left_joystick(context, 192, 255, 160, 0);
            pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);
        }
    )
    , Icelands_Arena(
        "icelands-arena",
        "Alabaster Icelands - Icepeak Arena",
        MapRegion::ICELANDS, 0, 2, nullptr, true
    )
    , Retreat(
        "retreat",
        "Ancient Retreat",
        MapRegion::RETREAT, 0, 0, nullptr, false
    )
    , Village_GalaxyHall(
        "village-galaxyhall",
        "Jubilife Village - Galaxy Hall",
        MapRegion::JUBILIFE, 0, 0, nullptr, false
    )
    , Village_FrontGate(
        "village-frontgate",
        "Jubilife Village - Front Gate",
        MapRegion::JUBILIFE, 0, 1, nullptr, false
    )
    , Village_PracticeField(
        "village-practicefield",
        "Jubilife Village - Practice Field",
        MapRegion::JUBILIFE, 0, 2, nullptr, false
    )
    , Village_Farm(
        "village-farm",
        "Jubilife Village - Farm",
        MapRegion::JUBILIFE, 0, 3, nullptr, false
    )
    , Village_TrainingGrounds(
        "village-traininggrounds",
        "Jubilife Village - Training Grounds",
        MapRegion::JUBILIFE, 0, 2, nullptr, true
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
    add_location(Icelands_PearlSettlement_SW);
    add_location(Icelands_Arena);
    add_location(Retreat);

    const bool inside_village = true;
    add_location(Village_GalaxyHall, inside_village);
    add_location(Village_FrontGate, inside_village);
    add_location(Village_PracticeField, inside_village);
    add_location(Village_Farm, inside_village);
    add_location(Village_TrainingGrounds, inside_village);
}









}
}
}
