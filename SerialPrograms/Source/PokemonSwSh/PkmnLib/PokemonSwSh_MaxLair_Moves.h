/*  Max Lair Moves
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef _PokemonAutomation_PokemonSwSh_MaxLair_Moves_H
#define _PokemonAutomation_PokemonSwSh_MaxLair_Moves_H

#include <string>
#include <map>
#include "Pokemon/Pokemon_Types.h"
#include "PokemonSwSh_MaxLair_Types.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{
using namespace Pokemon;

class Move{
public:
    Move() = default;
    Move(
        uint32_t id,
        std::string name,
        Type type,
        MoveCategory category,
        double base_power,
        double accuracy,
        uint8_t pp,
        std::string effect,
        double probability,
        bool is_spread,
        double correction_factor
    );

    // define getters
    uint32_t     id                 () const{ return m_id; }
    std::string  name               () const{ return m_name; }
    Type         type               () const{ return m_type; }
    MoveCategory category           () const{ return m_category; }
    double       base_power         () const{ return m_base_power; }
    double       accuracy           () const{ return m_accuracy; }
    uint8_t      pp                 () const{ return m_pp; }
    std::string  effect             () const{ return m_effect; }
    double       probability        () const{ return m_probability; }
    bool         is_spread          () const{ return m_is_spread; }
    double       correction_factor  () const{ return m_correction_factor; }

    // the only setter so far is updating the move type
    void move_type(Type update_type) { m_current_type = update_type; }
    void reset_move_type() { m_current_type = m_type; }

private:
    uint32_t        m_id            = 0;
    std::string     m_name          = "";
    Type            m_type          = Type::none;
    Type            m_current_type  = Type::none;
    MoveCategory    m_category      = MoveCategory::UNKNOWN;
    double          m_base_power    = 0.0;
    double          m_accuracy      = 0.0; // value between 0 and 1 for the accuracy
    uint8_t         m_pp            = 0;
    std::string     m_effect        = "";
    double          m_probability   = 0.0;
    bool            m_is_spread     = false;
    double          m_correction_factor = 0.0;
};


MoveCategory get_move_category_from_string(const std::string& category);

const std::map<uint32_t, Move>& all_moves();
const std::map<uint32_t, Move>& all_max_moves();

double get_max_move_power(const Move& move);



}
}
}
}
#endif
