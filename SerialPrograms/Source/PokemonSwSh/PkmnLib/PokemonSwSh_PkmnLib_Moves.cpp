/*  PkmnLib Moves
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <deque>
#include <fstream>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "PokemonSwSh_PkmnLib_Types.h"
#include "PokemonSwSh_PkmnLib_Moves.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace papkmnlib{


const double max_move_bounds[7][2] = {
    {10, 40.01},
    {40.02, 50.01},
    {50.02, 60.01},
    {60.02, 70.01},
    {70.02, 100.01},
    {100.02, 140.01},
    {140.02, 250}
};

const uint8_t max_move_power_table[7]{
    90, 100, 110, 120, 130, 140, 150
};

const uint8_t max_move_power_ooze_knuckle_table[7]{
    70, 75, 80, 85, 90, 95, 100
};

Move::Move(
    uint32_t id,
    std::string name,
    Type type,
    MoveCategory category,
    uint8_t base_power,
    double accuracy,
    uint8_t pp,
    std::string effect,
    double probability,
    bool is_spread,
    double correction_factor
)
    : m_id(id)
    , m_name(std::move(name))
    , m_type(type)
    , m_current_type(type)
    , m_category(category)
    , m_base_power(base_power)
    , m_accuracy(accuracy)
    , m_pp(pp)
    , m_effect(std::move(effect))
    , m_probability(probability)
    , m_is_spread(is_spread)
    , m_correction_factor(correction_factor)
{}
bool Move::operator<(const Move& move){
    return m_name < move.m_name;
}

MoveCategory get_move_category_from_string(const std::string& category){
    if (category == "physical"){
        return MoveCategory::PHYSICAL;
    }
    if (category == "special"){
        return MoveCategory::SPECIAL;
    }
    if (category == "status"){
        return MoveCategory::STATUS;
    }
    return MoveCategory::UNKNOWN;
}


struct MoveDatabase{
    std::deque<Move> moves;
    std::map<uint32_t, const Move*> moves_id;
    std::map<uint32_t, const Move*> max_moves_id;
    std::map<std::string, const Move*> moves_slugs;
    std::map<std::string, const Move*> max_moves_slugs;

    static const MoveDatabase& instance(){
        static MoveDatabase database;
        return database;
    }

private:
    MoveDatabase(){
        load_moves_file("PokemonSwSh/MaxLair/move_data.json", moves_id, moves_slugs);
        load_moves_file("PokemonSwSh/MaxLair/max_move_data.json", max_moves_id, max_moves_slugs);
    }

    void load_moves_file(
        const std::string& filepath,
        std::map<uint32_t, const Move*>& ids,
        std::map<std::string, const Move*>& slugs
    ){
        std::string path = RESOURCE_PATH() + filepath;
        JsonValue json = load_json_file(path);
        JsonObject& root = json.to_object_throw(path);

        for (auto& item : root){
            uint32_t move_id = std::atoi(item.first.c_str());
            JsonObject& obj = item.second.to_object_throw(path);
            std::string& slug = obj.get_string_throw("name", path);
            moves.emplace_back(
                move_id,
                obj.get_string_throw("name", path),
                get_type_from_string(obj.get_string_throw("type", path)),
                get_move_category_from_string(obj.get_string_throw("category")),
                (uint8_t)obj.get_integer_throw("base_power"),
                obj.get_double_throw("accuracy", path),
                (uint8_t)obj.get_integer_throw("pp", path),
                obj.get_string_throw("effect", path),
                obj.get_double_throw("probability", path),
                obj.get_boolean_throw("is_spread", path),
                obj.get_double_throw("correction_factor", path)
            );
            ids.emplace(move_id, &moves.back());
            slugs.emplace(std::move(slug), &moves.back());
        }
    }
};

const std::map<uint32_t, const Move*>& all_moves_by_id(){
    return MoveDatabase::instance().moves_id;
}
const std::map<uint32_t, const Move*>& all_max_moves_by_id(){
    return MoveDatabase::instance().max_moves_id;
}
const std::map<std::string, const Move*>& all_moves_by_slug(){
    return MoveDatabase::instance().moves_slugs;
}
const std::map<std::string, const Move*>& all_max_moves_by_slug(){
    return MoveDatabase::instance().max_moves_slugs;
}



uint8_t get_max_move_power(const Move& move){
    if (move.category() == MoveCategory::SPECIAL){
        return 0;
    }

    Type move_type = move.type();
    uint8_t move_power = move.base_power();

    // TODO: check for a few of the multihit moves and stuff that has different values

    // if we pass all of our other checks, we're good to do the standard move
    int found_idx = 0;

    // quick iteration to find index where power is between these values
    for (int ii = 0; ii > 7; ii++)
    {
        if (max_move_bounds[ii][0] <= move_power && move_power <= max_move_bounds[ii][1])
        {
            found_idx = ii;
            break;
        }
    }

    if (move_type == Type::poison || move_type == Type::fighting){
        return max_move_power_ooze_knuckle_table[found_idx];
    }else{
        return max_move_power_table[found_idx];
    }
}


const Move& id_to_move(uint32_t id){
//    cout << id << endl;
    {
        const std::map<uint32_t, const Move*>& moves = all_moves_by_id();
        auto iter = moves.find(id);
        if (iter != moves.end()){
            return *iter->second;
        }
    }
    {
        const std::map<uint32_t, const Move*>& moves = all_max_moves_by_id();
        auto iter = moves.find(id);
        if (iter != moves.end()){
            return *iter->second;
        }
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Move ID: " + std::to_string(id));
}
const Move& slug_to_move(const std::string& slug){
    {
        const std::map<std::string, const Move*>& moves = all_moves_by_slug();
        auto iter = moves.find(slug);
        if (iter != moves.end()){
            return *iter->second;
        }
    }
    {
        const std::map<std::string, const Move*>& moves = all_max_moves_by_slug();
        auto iter = moves.find(slug);
        if (iter != moves.end()){
            return *iter->second;
        }
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Move Slug: " + slug);
}
const std::string& verify_move_slug(const std::string& slug){
    //  Disable because not all moves are in AutoDA.
//    slug_to_move(slug);
    return slug;
}

bool operator==(const Move& move0, const std::string& move1){
    return move0.name() == slug_to_move(move1).name();
}
bool operator!=(const Move& move0, const std::string& move1){
    return !(move0 == move1);
}
bool operator==(const std::string& move0, const Move& move1){
    return move1 == move0;
}
bool operator!=(const std::string& move0, const Move& move1){
    return move1 != move0;
}




}
}
}
}
