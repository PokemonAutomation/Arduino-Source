/*  PkmnLib Field
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef _PokemonAutomation_PokemonSwSh_PkmnLib_Field_H
#define _PokemonAutomation_PokemonSwSh_PkmnLib_Field_H

#include <string>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{


enum class Weather{
    CLEAR,
    RAIN,
    SUN,
    SANDSTORM,
    HAIL
};

enum class Terrain{
    NONE,
    ELECTRIC,
    GRASSY,
    MISTY,
    PSYCHIC
};


class Field{
    Weather m_default_weather = Weather::CLEAR;
    Weather m_current_weather = m_default_weather;

    Terrain m_default_terrain = Terrain::NONE;
    Terrain m_current_terrain = m_default_terrain;

public:
    Field() = default;
    Field(Weather w, Terrain t)
        : m_default_weather(w)
        , m_current_weather(w)
        , m_default_terrain(t)
        , m_current_terrain(t)
    {}

    // weather related-queries
    Weather weather() const{ return m_current_weather; }
    bool is_clear() const{ return m_current_weather == Weather::CLEAR; }
    bool is_rain() const{ return m_current_weather == Weather::RAIN; }
    bool is_sun() const{ return m_current_weather == Weather::SUN; }
    bool is_sandstorm() const{ return m_current_weather == Weather::SANDSTORM; }
    bool is_hail() const{ return m_current_weather == Weather::HAIL; }

    // set the weather
    void set_rain() { m_current_weather = Weather::RAIN; }
    void set_sun() { m_current_weather = Weather::SUN; }
    void set_sandstorm() { m_current_weather = Weather::SANDSTORM; }
    void set_hail() { m_current_weather = Weather::HAIL; }
    void set_clear() { m_current_weather = Weather::CLEAR; }

    // terrain-related queries
    Terrain terrain() const{ return m_current_terrain; }
    bool is_electric() const{ return m_current_terrain == Terrain::ELECTRIC; }
    bool is_grassy() const{ return m_current_terrain == Terrain::GRASSY; }
    bool is_misty() const{ return m_current_terrain == Terrain::MISTY; }
    bool is_psychic() const{ return m_current_terrain == Terrain::PSYCHIC; }
    bool is_none_terrain() const{ return m_current_terrain == Terrain::NONE; }

    // set the terrain
    void set_electric() { m_current_terrain = Terrain::ELECTRIC; }
    void set_grassy() { m_current_terrain = Terrain::GRASSY; }
    void set_misty() { m_current_terrain = Terrain::MISTY; }
    void set_psychic() { m_current_terrain = Terrain::PSYCHIC; }
    void set_none_terrain() { m_current_terrain = Terrain::NONE; }

    // reset field
    void reset(){
        m_current_terrain = m_default_terrain;
        m_current_weather = m_default_weather;
    }

    void set_default_field(const std::string& boss);
};





}
}
}
}
#endif
