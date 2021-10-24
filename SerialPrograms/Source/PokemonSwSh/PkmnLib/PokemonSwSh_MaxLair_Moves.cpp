/*  Max Lair Moves
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <fstream>
#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "PokemonSwSh_MaxLair_Types.h"
#include "PokemonSwSh_MaxLair_Moves.h"

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

const double max_move_power_table[7]{
    90.0, 100.0, 110.0, 120.0, 130.0, 140.0, 150.0
};

const double max_move_power_ooze_knuckle_table[7]{
    70.0, 75.0, 80.0, 85.0, 90.0, 95.0, 100.0
};

Move::Move(
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


std::map<uint32_t, Move> parse_moves_file(const std::string& filepath){
    QString path = RESOURCE_PATH() + QString::fromStdString(filepath);
    QJsonObject json = read_json_file(path).object();
    if (json.empty()){
        PA_THROW_FileException("Json is either empty or invalid.", path);
    }

    std::map<uint32_t, Move> moves;

    for (auto iter = json.begin(); iter != json.end(); ++iter){
        uint32_t move_id = iter.key().toUInt();
        QJsonObject obj = iter.value().toObject();
        moves.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(move_id),
            std::forward_as_tuple(
                move_id,
                json_get_string_throw(obj, "name").toStdString(),
                get_type_from_string(json_get_string_throw(obj, "type").toStdString()),
                get_move_category_from_string(json_get_string_throw(obj, "category").toStdString()),
                json_get_double_throw(obj, "base_power"),
                json_get_double_throw(obj, "accuracy"),
                json_get_int_throw(obj, "pp"),
                json_get_string_throw(obj, "effect").toStdString(),
                json_get_double_throw(obj, "probability"),
                json_get_bool_throw(obj, "is_spread"),
                json_get_double_throw(obj, "correction_factor")
            )
        );
    }

    return moves;
}

const std::map<uint32_t, Move>& all_moves(){
    static std::map<uint32_t, Move> moves = parse_moves_file("PokemonSwSh/MaxLair/move_data.json");
    return moves;
}
const std::map<uint32_t, Move>& all_max_moves(){
    static std::map<uint32_t, Move> moves = parse_moves_file("PokemonSwSh/MaxLair/max_move_data.json");
    return moves;
}



double get_max_move_power(const Move& move){
    if (move.category() == MoveCategory::SPECIAL){
        return 0;
    }

    Type move_type = move.type();
    double move_power = move.base_power();

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




}
}
}
}
