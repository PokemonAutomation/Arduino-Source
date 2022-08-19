/*  Travel Locations
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_TravelLocations_H
#define PokemonAutomation_PokemonLA_TravelLocations_H

#include <functional>
#include <string>
#include <vector>
#include <map>
#include "Common/Cpp/EnumDatabase.h"
#include "ClientSource/Connection/BotBase.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA_Locations.h"

namespace PokemonAutomation{
    class ConsoleHandle;
namespace NintendoSwitch{
namespace PokemonLA{



struct TravelLocation{
    const char* slug;
    const char* display;

    MapRegion region;
    uint8_t warp_slot;
    uint8_t warp_sub_slot;

    std::function<void(ConsoleHandle& console, BotBaseContext& context)> post_arrival_maneuver;

    TravelLocation(
        const char* p_slug, const char* p_display,
        MapRegion p_region,
        uint8_t p_warp_slot, uint8_t p_warp_sub_slot,
        std::function<void(ConsoleHandle& console, BotBaseContext& context)>&& p_post_arrival_maneuver
    );
};


class TravelLocations{
public:
    static const TravelLocations& instance();

    const TravelLocation& operator[](size_t index) const{
        return *m_list[index];
    }
    const IntegerEnumDatabase& database() const;


public:
    const TravelLocation Fieldlands_Fieldlands;
    const TravelLocation Fieldlands_Heights;
    const TravelLocation Fieldlands_Arena;

    const TravelLocation Mirelands_Mirelands;
    const TravelLocation Mirelands_Bogbound;
    const TravelLocation Mirelands_DiamondSettlement;
    const TravelLocation Mirelands_Arena;

    const TravelLocation Coastlands_Beachside;
    const TravelLocation Coastlands_Coastlands;
    const TravelLocation Coastlands_Arena;
    const TravelLocation Coastlands_Arena_NW;

    const TravelLocation Highlands_Highlands;
    const TravelLocation Highlands_Mountain;
    const TravelLocation Highlands_Summit;
    const TravelLocation Highlands_Arena;

    const TravelLocation Icelands_Snowfields;
    const TravelLocation Icelands_Icepeak;
    const TravelLocation Icelands_PearlSettlement;
    const TravelLocation Icelands_Arena;


private:
    TravelLocations();
    void add_location(const TravelLocation& location);

    std::vector<const TravelLocation*> m_list;
    std::map<std::string, const TravelLocation*> m_map;

    IntegerEnumDatabase m_database;
};






}
}
}
#endif
