/*  PkmnLib Pokemon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef _PokemonAutomation_PokemonSwSh_PkmnLib_Pokemon_H
#define _PokemonAutomation_PokemonSwSh_PkmnLib_Pokemon_H

#include <string>
#include <map>
#include "PokemonSwSh_PkmnLib_Types.h"
#include "PokemonSwSh_PkmnLib_Stats.h"
#include "PokemonSwSh_PkmnLib_Moves.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{


enum class NonVolatileStatusEffects{
    NO_NON_VOLATILE_STATUS,
    BURN,
    FREEZE,
    PARALYSIS,
    POISON,
    SLEEP
};

enum class VolatileStatusEffects{
    NO_VOLATILE_STATUS,
    CONFUSION,
    BOUND,
    CANT_ESCAPE,
    CURSE,
    EMBARGO,
    ENCORE,
    FLINCH,
    HEAL_BLOCK,
    IDENTIFIED,
    INFATUATION,
    LEECH_SEED,
    NIGHTMARE,
    PERISH_SONG,
    TAUNT,
    TELEKINESIS,
    TORMENT
};

class Pokemon{
public:
    // declare public functions now

    // constructor without move information (note that this leaves garbage in move information!)
    Pokemon(
        uint16_t dex_id,
        std::string name,
        std::string ability,
        Type type1,
        Type type2,
        uint8_t level,
        uint8_t base_hp,
        uint8_t base_atk,
        uint8_t base_def,
        uint8_t base_spatk,
        uint8_t base_spdef,
        uint8_t base_speed
    );

    // constructor with all move IDs as well
    Pokemon(
        uint16_t dex_id,
        std::string name,
        std::string ability,
        Type type1,
        Type type2,
        uint8_t level,
        uint8_t base_hp,
        uint8_t base_atk,
        uint8_t base_def,
        uint8_t base_spatk,
        uint8_t base_spdef,
        uint8_t base_speed,
        uint32_t move1_id,
        uint32_t move2_id,
        uint32_t move3_id,
        uint32_t move4_id,
        uint32_t legendary_desparation_move_id,
        uint32_t max_move1_id,
        uint32_t max_move2_id,
        uint32_t max_move3_id,
        uint32_t max_move4_id,
        uint32_t max_move5_id
    );

    // declare basic getters for some of the values we have
    uint16_t            dex_id() const{ return m_dex_id; }
    const std::string&  name() const{ return m_name; }
    const std::string&  ability() const{ return m_ability; }
    Type                type1() const{ return m_type1; }
    Type                type2() const{ return m_type2; }
    uint8_t             level() const{ return m_level; }
    uint16_t            max_hp() const{ return m_max_hp; }
    uint16_t            current_hp() const{ return m_current_hp; }
    uint16_t            attack() const{ return m_calc_atk; }
    uint16_t            defense() const{ return m_calc_def; }
    uint16_t            special_attack() const{ return m_calc_spatk; }
    uint16_t            special_defense() const{ return m_calc_spdef; }
    uint16_t            speed() const{ return m_calc_speed; }

    bool is_dynamax() const{ return m_is_dynamax; }
    void set_is_dynamax(bool is_dynamax) { m_is_dynamax = is_dynamax; }

    bool is_stab(Type move_type) const{ return move_type == m_type1 || move_type == m_type2; }
    bool has_type(Type theType) const{ return theType == m_type1 || theType == m_type2; }

    bool is_legendary() const{ return m_is_legendary; }
    void set_is_legendary(bool is_legendary) { m_is_legendary = is_legendary; }

    size_t num_moves() const;
    const Move& move(size_t index) const;
    const Move& max_move(size_t index) const;

    // function for setting the pointers to the moves
    void set_move(const Move& move, size_t index);
    void set_max_move(const Move& move, size_t index);

    uint32_t move_id(size_t index) const;
    uint32_t max_move_id(size_t index) const;

    const Move& desparation_move() const{ return *m_move[4]; }
    uint8_t pp(size_t index) const;
    void update_pp(size_t index, int8_t movePP);
//    void reduce_pp(size_t index, int8_t amount = 1);
//    void reset_pp();

    // functions that need their methods defined
    void update_stats(bool is_iv, uint8_t HP, uint8_t Atk, uint8_t Def, uint8_t SpAtk, uint8_t SpDef, uint8_t Speed);
    void update_stats(
        uint8_t iv_hp, uint8_t iv_atk, uint8_t iv_def, uint8_t iv_spatk, uint8_t iv_spdef, uint8_t iv_speed,
        uint8_t ev_hp, uint8_t ev_atk, uint8_t ev_def, uint8_t ev_spatk, uint8_t ev_spdef, uint8_t ev_speed
    );
    void reset_hp();
    double hp_ratio() const;
    void set_hp_ratio(double hp_ratio);
//    void take_damage(uint16_t damage);
//    void heal(uint16_t points);
//    void set_level(uint8_t level) { m_level = level; }

    VolatileStatusEffects volatile_status_effect() const{ return volatile_status; }
    NonVolatileStatusEffects non_volatile_status_effect() const{ return non_volatile_status; }

    void set_volatile_status_effect(VolatileStatusEffects inVar){ volatile_status = inVar; }
    void set_non_volatile_status_effect(NonVolatileStatusEffects inVar){ non_volatile_status = inVar; }
    void clear_volatile_status_effect(){ volatile_status = VolatileStatusEffects::NO_VOLATILE_STATUS; }
    void clear_non_volatile_status_effect(){ non_volatile_status = NonVolatileStatusEffects::NO_NON_VOLATILE_STATUS; }

    void transform_from_ditto(const Pokemon& opponent);

    std::string dump() const;

private:
    void assert_move_index(size_t index) const;

private:
    // private members, shouldn't need to be accessed unless with accessors

    // basic information regarding the pokemon itself
    uint16_t m_dex_id;
    std::string m_name;
    std::string m_ability;
    // pokemon types, uses the enum Type from Types.h
    Type m_type1 = Type::none;
    Type m_type2 = Type::none;
    uint8_t m_level;

    uint8_t m_base_hp, m_base_atk, m_base_def, m_base_spatk, m_base_spdef, m_base_speed;

    // IVs that we're to use in calculations
    uint8_t m_iv_hp, m_iv_atk, m_iv_def, m_iv_spatk, m_iv_spdef, m_iv_speed;
    // EVs that we're to use in calculations
    uint8_t m_ev_hp, m_ev_atk, m_ev_def, m_ev_spatk, m_ev_spdef, m_ev_speed;

    // calculated HP, will be determined based on input information during initialization
    uint16_t m_max_hp, m_calc_atk, m_calc_def, m_calc_spatk, m_calc_spdef, m_calc_speed;
    uint16_t m_current_hp;

    bool m_is_legendary = false;

    // move information
    size_t m_num_moves;
    uint32_t m_move_id[5] = {0, 0, 0, 0, 0};

    // pointers to store the move information for convenience
    const Move* m_move[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};

    // move PP - note that they are *shared* with dynamax PP
    int8_t m_pp[5] = {0, 0, 0, 0, 0};

    bool m_is_dynamax = false;

    uint32_t m_max_move_id[5] = {0, 0, 0, 0, 0};
    const Move* m_max_move[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};

    void calculate_stats();
    void load_moves();

    VolatileStatusEffects volatile_status = VolatileStatusEffects::NO_VOLATILE_STATUS;
    NonVolatileStatusEffects non_volatile_status = NonVolatileStatusEffects::NO_NON_VOLATILE_STATUS;
};


const std::map<std::string, Pokemon>& all_rental_pokemon();
const std::map<std::string, Pokemon>& all_boss_pokemon();

const Pokemon& get_pokemon(const std::string& slug);



}
}
}
}
#endif
