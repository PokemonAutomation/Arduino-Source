/*  PkmnLib Pokemon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "PokemonSwSh_PkmnLib_Pokemon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{


// default iv values, the "average" IV is 26, since there'a a 4/6 chance to get 31 and 2/6 chance to get 0-31
#define default_iv 26
// default ev is 0, none of the pokemon are actually "trained"
#define default_ev 0
// the default level for a pokemon is 65
#define default_level 65
// default level for a legendary pokemon is 70
#define default_legendary_level 70



Pokemon::Pokemon(
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
)
    : m_dex_id(dex_id)
    , m_name(std::move(name))
    , m_ability(std::move(ability))
    , m_type1(type1)
    , m_type2(type2)
    , m_level(level)
    , m_base_hp(base_hp)
    , m_base_atk(base_atk)
    , m_base_def(base_def)
    , m_base_spatk(base_spatk)
    , m_base_spdef(base_spdef)
    , m_base_speed(base_speed)
{
    load_moves();

    // initialize the stats based on the defaults
    m_iv_hp = m_iv_atk = m_iv_def = m_iv_spatk = m_iv_spdef = m_iv_speed = default_iv;

    // initialize the default EVs as well
    m_ev_hp = m_ev_atk = m_ev_def = m_ev_spatk = m_ev_spdef = m_ev_speed = default_ev;

    // then we need to calculate our stats
    calculate_stats();
}
Pokemon::Pokemon(
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
)
    : m_dex_id(dex_id)
    , m_name(std::move(name))
    , m_ability(std::move(ability))
    , m_type1(type1)
    , m_type2(type2)
    , m_level(level)
    , m_base_hp(base_hp)
    , m_base_atk(base_atk)
    , m_base_def(base_def)
    , m_base_spatk(base_spatk)
    , m_base_spdef(base_spdef)
    , m_base_speed(base_speed)
{
    m_move_id[0] = move1_id;
    m_move_id[1] = move2_id;
    m_move_id[2] = move3_id;
    m_move_id[3] = move4_id;
    m_move_id[4] = legendary_desparation_move_id;

    m_max_move_id[0] = max_move1_id;
    m_max_move_id[1] = max_move2_id;
    m_max_move_id[2] = max_move3_id;
    m_max_move_id[3] = max_move4_id;
    m_max_move_id[4] = max_move5_id;

    load_moves();

    // initialize the stats based on the defaults
    m_iv_hp = m_iv_atk = m_iv_def = m_iv_spatk = m_iv_spdef = m_iv_speed = default_iv;

    // initialize the default EVs as well
    m_ev_hp = m_ev_atk = m_ev_def = m_ev_spatk = m_ev_spdef = m_ev_speed = default_ev;

    // then we need to calculate our stats
    calculate_stats();
}

void Pokemon::calculate_stats(){
    // this method calculates stats from whatever is internally stored, use the update methods
    // if we want to modify IVs or EVs

    // HP stat
    m_max_hp = calc_hp(m_base_hp, m_iv_hp, m_ev_hp, m_level);
    // make sure we store the current HP
    m_current_hp = m_max_hp;

    // physical stats
    m_calc_atk = calc_stat(m_base_atk, m_iv_atk, m_ev_atk, m_level);
    m_calc_def = calc_stat(m_base_def, m_iv_def, m_ev_def, m_level);

    // special stats
    m_calc_spatk = calc_stat(m_base_spatk, m_iv_spatk, m_ev_spatk, m_level);
    m_calc_spdef = calc_stat(m_base_spdef, m_iv_spdef, m_ev_spdef, m_level);

    // speed stat
    m_calc_speed = calc_stat(m_base_speed, m_iv_speed, m_ev_speed, m_level);
}
void Pokemon::load_moves(){
    for (size_t c = 0; c < 5; c++){
        if (m_move_id[c] != 0){
            m_move[c] = &id_to_move(m_move_id[c]);
            m_pp[c] = m_move[c]->pp();
        }
        if (m_max_move_id[c] != 0){
            m_max_move[c] = &id_to_move(m_max_move_id[c]);
        }
    }

    do{
        if (is_legendary()){
            m_num_moves = 5;
            break;
        }
        size_t c = 0;
        for (; c < 4; c++){
            if (m_move[c] == nullptr){
                break;
            }
        }
        m_num_moves = c;
    }while (false);
}

void Pokemon::update_stats(bool is_iv, uint8_t HP, uint8_t Atk, uint8_t Def, uint8_t SpAtk, uint8_t SpDef, uint8_t Speed){
    // this method updates internally what the IVs or EVs are and then recalculates stats
    if (is_iv){
        m_iv_hp = HP;
        m_iv_atk = Atk;
        m_iv_def = Def;
        m_iv_spatk = SpAtk;
        m_iv_spdef = SpDef;
        m_iv_speed = Speed;
    }else{
        m_ev_hp = HP;
        m_ev_atk = Atk;
        m_ev_def = Def;
        m_ev_spatk = SpAtk;
        m_ev_spdef = SpDef;
        m_ev_speed = Speed;
    }

    // then recalculate stats
    calculate_stats();
}
void Pokemon::update_stats(
    uint8_t iv_hp, uint8_t iv_atk, uint8_t iv_def, uint8_t iv_spatk, uint8_t iv_spdef, uint8_t iv_speed,
    uint8_t ev_hp, uint8_t ev_atk, uint8_t ev_def, uint8_t ev_spatk, uint8_t ev_spdef, uint8_t ev_speed
){
    // this method updates internally what the IVs and EVs are and then recalculates stats

    // update the IVs
    m_iv_hp = iv_hp;
    m_iv_atk = iv_atk;
    m_iv_def = iv_def;
    m_iv_spatk = iv_spatk;
    m_iv_spdef = iv_spdef;
    m_iv_speed = iv_speed;

    // update the EVs
    m_ev_hp = ev_hp;
    m_ev_atk = ev_atk;
    m_ev_def = ev_def;
    m_ev_spatk = ev_spatk;
    m_ev_spdef = ev_spdef;
    m_ev_speed = ev_speed;

    // then recalculate stats
    calculate_stats();
}


void Pokemon::reset_hp(){
    m_current_hp = m_max_hp;
}
double Pokemon::hp_ratio() const{
    return (double)m_current_hp / m_max_hp;
}
void Pokemon::set_hp_ratio(double hp_ratio){
    m_current_hp = (uint16_t)(m_max_hp * hp_ratio);
}
#if 0
void Pokemon::take_damage(uint16_t damage){
    m_current_hp = damage > m_current_hp
        ? 0
        : m_current_hp - damage;
}
void Pokemon::heal(uint16_t points){
    m_current_hp += points;
    m_current_hp = std::min(m_current_hp, m_max_hp);
}
#endif

void Pokemon::assert_move_index(size_t index) const{
    if (index < num_moves()){
        return;
    }
    throw InternalProgramError(
        nullptr, PA_CURRENT_FUNCTION,
        "Move index out-of-range: mon = " + m_name + ", index = " + std::to_string(index)
    );
}

size_t Pokemon::num_moves() const{
    return m_num_moves;
}
const Move& Pokemon::move(size_t index) const{
    assert_move_index(index);
    return *m_move[index];
}
const Move& Pokemon::max_move(size_t index) const{
    assert_move_index(index);
    return *m_max_move[index];
}
void Pokemon::set_move(const Move& move, size_t index){
    assert_move_index(index);
    m_move[index] = &move;
}
void Pokemon::set_max_move(const Move& move, size_t index){
    assert_move_index(index);
    m_max_move[index] = &move;
}

uint32_t Pokemon::move_id(size_t index) const{
    assert_move_index(index);
    return m_move_id[index];
}
uint32_t Pokemon::max_move_id(size_t index) const{
    assert_move_index(index);
    return m_max_move_id[index];
}

uint8_t Pokemon::pp(size_t index) const{
    assert_move_index(index);
    return m_pp[index];
}
void Pokemon::update_pp(size_t index, int8_t movePP){
    assert_move_index(index);
    m_pp[index] = std::min(movePP, (int8_t)m_move[index]->pp());
}
#if 0
void Pokemon::reduce_pp(size_t index, int8_t amount){
    assert_move_index(index);
    m_pp[index] -= amount;
    m_pp[index] = std::min(m_pp[index], (int8_t)m_move[index]->pp());
    m_pp[index] = std::max(m_pp[index], (int8_t)0);
}
void Pokemon::reset_pp(){
    for (size_t c = 0; c < 5; c++){
        const Move* move = m_move[c];
        m_pp[c] = move == nullptr ? 0 : move->pp();
    }
}
#endif

void Pokemon::transform_from_ditto(const Pokemon& opponent){
    if (m_name != "ditto"){
        return;
    }

    double hp_ratio = (double)m_current_hp / m_max_hp;
    bool dmaxed = m_is_dynamax;

    *this = opponent;
    m_pp[0] = 5;
    m_pp[1] = 5;
    m_pp[2] = 5;
    m_pp[3] = 5;

    set_hp_ratio(hp_ratio);
    set_is_dynamax(dmaxed);
}

std::string Pokemon::dump() const{
    std::string str = m_name + ": ";
    str += "hp:" + std::to_string(m_current_hp) + "/" + std::to_string(m_max_hp);
    str += ", dmax:" + std::to_string(m_is_dynamax);
    str += "\n";
    return str;
}


std::map<std::string, Pokemon> load_pokemon(const std::string& filepath, bool is_legendary){
    std::string path = RESOURCE_PATH() + filepath;
    JsonValue json = load_json_file(path);
    JsonObject& root = json.to_object_throw(path);

    std::map<std::string, Pokemon> map;

    uint8_t level;
    size_t move_limit;
    if (is_legendary){
        level = default_legendary_level;
        move_limit = 5;
    }else{
        level = default_level;
        move_limit = 4;
    }

    for (auto& item : root){
        const std::string& slug = item.first;
        JsonObject& obj = item.second.to_object_throw(path);

        Type type1 = Type::none;
        Type type2 = Type::none;
        {
            JsonArray& array = obj.get_array_throw("types", path);
            if (array.size() < 1){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "Must have at least one type: " + slug, std::move(path));
            }
            type1 = get_type_from_string(array[0].to_string_throw(path));
            if (array.size() > 1){
                type2 = get_type_from_string(array[1].to_string_throw(path));
            }
        }

        uint32_t moves[5] = {0, 0, 0, 0, 0};
        {
            JsonArray& array = obj.get_array_throw("moves", path);
            if (array.size() > move_limit){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "Too many moves specified: " + slug, std::move(path));
            }
            for (size_t c = 0; c < array.size(); c++){
                moves[c] = (uint32_t)array[c].to_integer_throw(path);
            }
        }

        uint32_t max_moves[5] = {0, 0, 0, 0, 0};
        {
            JsonArray& array = obj.get_array_throw("max_moves", path);
            if (array.size() > move_limit){
                throw FileException(nullptr, PA_CURRENT_FUNCTION, "Too many moves specified: " + slug, std::move(path));
            }
            for (size_t c = 0; c < array.size(); c++){
                max_moves[c] = (uint32_t)array[c].to_integer_throw(path);
            }
        }
        JsonArray& base_stats = obj.get_array_throw("base_stats", path);
        if (base_stats.size() != 6){
            throw FileException(nullptr, PA_CURRENT_FUNCTION, "Should be exactly 6 base stats: " + slug, std::move(path));
        }

        map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(slug),
            std::forward_as_tuple(
                (uint16_t)obj.get_integer_throw("id", path),
                slug,
                obj.get_string_throw("ability_id", path),
                type1, type2,
                level,
                (uint8_t)base_stats[0].to_integer_throw(path),
                (uint8_t)base_stats[1].to_integer_throw(path),
                (uint8_t)base_stats[2].to_integer_throw(path),
                (uint8_t)base_stats[3].to_integer_throw(path),
                (uint8_t)base_stats[4].to_integer_throw(path),
                (uint8_t)base_stats[5].to_integer_throw(path),
                moves[0],
                moves[1],
                moves[2],
                moves[3],
                moves[4],
                max_moves[0],
                max_moves[1],
                max_moves[2],
                max_moves[3],
                max_moves[4]
            )
        );
    }

    return map;
}


const std::map<std::string, Pokemon>& all_rental_pokemon(){
    static std::map<std::string, Pokemon> pokemon = load_pokemon("PokemonSwSh/MaxLair/rental_pokemon.json", false);
    return pokemon;
}
const std::map<std::string, Pokemon>& all_boss_pokemon(){
    static std::map<std::string, Pokemon> pokemon = load_pokemon("PokemonSwSh/MaxLair/boss_pokemon.json", true);
    return pokemon;
}

const Pokemon& get_pokemon(const std::string& slug){
    {
        const std::map<std::string, Pokemon>& database = all_rental_pokemon();
        auto iter = database.find(slug);
        if (iter != database.end()){
            return iter->second;
        }
    }
    {
        const std::map<std::string, Pokemon>& database = all_boss_pokemon();
        auto iter = database.find(slug);
        if (iter != database.end()){
            return iter->second;
        }
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Slug not found: " + slug);
}


















}
}
}
}
