/*  Seeds Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "CommonFramework/Globals.h"
#include "PokemonFRLG_SeedsDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


std::string seeds_json_path(bool firered, Language language){
    std::string version = firered ? "FR" : "LG";
    std::string lang = (language == Language::Japanese) ? "Jpn" : "Eng";
    return "PokemonFRLG/RngSeeds/" + version + "_" + lang + ".json";
}


namespace{

uint16_t parse_hex_seed(const std::string& text, const std::string& path){
    try{
        size_t consumed = 0;
        unsigned long value = std::stoul(text, &consumed, 16);
        if (consumed != text.size() || value > 0xFFFF){
            throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid seed value: " + text, path);
        }
        return uint16_t(value);
    }catch (const std::invalid_argument&){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid seed value: " + text, path);
    }catch (const std::out_of_range&){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid seed value: " + text, path);
    }
}

// Column names have the form "<Sound> / <Button Mode> / <Seed Button>"
void parse_column_name(
    const std::string& name, const std::string& path,
    SoundSetting& sound, std::string& button_mode, SeedButton& seed_button
){
    std::vector<std::string> parts;
    size_t start = 0;
    while (true){
        size_t sep = name.find(" / ", start);
        if (sep == std::string::npos){
            parts.push_back(name.substr(start));
            break;
        }
        parts.push_back(name.substr(start, sep - start));
        start = sep + 3;
    }
    if (parts.size() != 3){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Malformed seed column name: " + name, path);
    }

    if (parts[0] == "Mono"){
        sound = SoundSetting::Mono;
    }else if (parts[0] == "Stereo"){
        sound = SoundSetting::Stereo;
    }else{
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unknown sound setting in column: " + name, path);
    }

    button_mode = parts[1];

    if (parts[2] == "A"){
        seed_button = SeedButton::A;
    }else if (parts[2] == "START" || parts[2] == "Start"){
        seed_button = SeedButton::Start;
    }else if (parts[2] == "L"){
        seed_button = SeedButton::L;
    }else{
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Unknown seed button in column: " + name, path);
    }
}

}


SeedsDatabase::SeedsDatabase(const std::string& json_path){
    std::string path = RESOURCE_PATH() + json_path;
    JsonValue json = load_json_file(path);
    JsonObject& root = json.to_object_throw(path);

    JsonArray& delays = root.get_array_throw("Delay", path);
    m_delays.reserve(delays.size());
    for (auto& delay : delays){
        m_delays.emplace_back(uint64_t(delay.to_integer_throw(path)));
    }

    for (auto& item : root){
        const std::string& name = item.first;
        if (name == "Delay"){
            continue;
        }
        SeedColumn column;
        column.name = name;
        parse_column_name(name, path, column.sound, column.button_mode, column.seed_button);

        JsonArray& seeds = item.second.to_array_throw(path);
        column.seeds.reserve(seeds.size());
        for (auto& seed : seeds){
            column.seeds.emplace_back(parse_hex_seed(seed.to_string_throw(path), path));
        }
        m_columns.emplace_back(std::move(column));
    }
}


SeedMatch SeedsDatabase::find_seed(uint16_t target_seed, SoundSetting sound, int radius) const{
    struct Variant{
        BlackoutButton extra_button;
        uint16_t search_value;
    };
    const Variant variants[] = {
        {BlackoutButton::None, target_seed},
        {BlackoutButton::L, uint16_t(target_seed + BLACKOUT_SEED_OFFSET)},
    };

    SeedMatch best;
    uint64_t best_delay = 0xFFFFFFFFFFFFFFFF;

    for (const SeedColumn& column : m_columns){
        if (column.sound != sound){
            continue;
        }
        // Only the "Help" button mode is supported: seed lists for other button modes
        // are almost entirely empty, and it is not exposed as a user setting.
        if (column.button_mode != "Help"){
            continue;
        }
        for (const Variant& variant : variants){
            for (size_t i = 0; i < column.seeds.size() && i < m_delays.size(); i++){
                if (column.seeds[i] != variant.search_value){
                    continue;
                }
                if (m_delays[i] >= best_delay){
                    break;
                }

                size_t list_start = (i > (size_t)radius) ? i - (size_t)radius : 0;
                size_t list_end = std::min(column.seeds.size() - 1, i + (size_t)radius);
                std::vector<uint16_t> seed_values;
                seed_values.reserve(list_end - list_start + 1);
                for (size_t j = list_start; j <= list_end; j++){
                    uint16_t value = column.seeds[j];
                    if (variant.extra_button != BlackoutButton::None){
                        value = uint16_t(value - BLACKOUT_SEED_OFFSET);
                    }
                    seed_values.emplace_back(value);
                }

                best_delay = m_delays[i];
                best.found = true;
                best.seed_values = std::move(seed_values);
                best.seed_position = int16_t(i - list_start);
                best.seed_delay = m_delays[i];
                best.seed_button = column.seed_button;
                best.extra_button = variant.extra_button;
                best.sound = column.sound;
                best.button_mode = column.button_mode;
                best.column_name = column.name;
                break;
            }
        }
    }

    return best;
}



}
}
}
