/*  Pokemon Nature Checker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Pokemon_NatureChecker.h"

namespace PokemonAutomation{
namespace Pokemon{

const std::map<std::string, NatureCheckerValue>& NatureCheckerValue_TOKEN_TO_ENUM(){
    static std::map<std::string, NatureCheckerValue> data{
        {"Adamant",     NatureCheckerValue::Adamant},
        {"Bashful",     NatureCheckerValue::Bashful},
        {"Bold",        NatureCheckerValue::Bold},
        {"Brave",       NatureCheckerValue::Brave},
        {"Calm",        NatureCheckerValue::Calm},
        {"Careful",     NatureCheckerValue::Careful},
        {"Docile",      NatureCheckerValue::Docile},
        {"Gentle",      NatureCheckerValue::Gentle},
        {"Hardy",       NatureCheckerValue::Hardy},
        {"Hasty",       NatureCheckerValue::Hasty},
        {"Impish",      NatureCheckerValue::Impish},
        {"Jolly",       NatureCheckerValue::Jolly},
        {"Lax",         NatureCheckerValue::Lax},
        {"Lonely",      NatureCheckerValue::Lonely},
        {"Mild",        NatureCheckerValue::Mild},
        {"Modest",      NatureCheckerValue::Modest},
        {"Naive",       NatureCheckerValue::Naive},
        {"Naughty",     NatureCheckerValue::Naughty},
        {"Quiet",       NatureCheckerValue::Quiet},
        {"Quirky",      NatureCheckerValue::Quirky},
        {"Rash",        NatureCheckerValue::Rash},
        {"Relaxed",     NatureCheckerValue::Relaxed},
        {"Sassy",       NatureCheckerValue::Sassy},
        {"Serious",     NatureCheckerValue::Serious},
        {"Timid",       NatureCheckerValue::Timid},
    };
    return data;
}
const std::map<NatureCheckerValue, std::string>& NatureCheckerValue_ENUM_TO_TOKEN(){
    static std::map<NatureCheckerValue, std::string> data{
        {NatureCheckerValue::UnableToDetect,    "Unable to Detect"},
        {NatureCheckerValue::Neutral,           "Neutral"},
        {NatureCheckerValue::Adamant,           "Adamant"},
        {NatureCheckerValue::Bashful,           "Bashful"},
        {NatureCheckerValue::Bold,              "Bold"},
        {NatureCheckerValue::Brave,             "Brave"},
        {NatureCheckerValue::Calm,              "Calm"},
        {NatureCheckerValue::Careful,           "Careful"},
        {NatureCheckerValue::Gentle,            "Gentle"},
        {NatureCheckerValue::Hardy,             "Hardy"},
        {NatureCheckerValue::Hasty,             "Hasty"},
        {NatureCheckerValue::Impish,            "Impish"},
        {NatureCheckerValue::Jolly,             "Jolly"},
        {NatureCheckerValue::Lax,               "Lax"},
        {NatureCheckerValue::Lonely,            "Lonely"},
        {NatureCheckerValue::Mild,              "Mild"},
        {NatureCheckerValue::Modest,            "Modest"},
        {NatureCheckerValue::Naive,             "Naive"},
        {NatureCheckerValue::Naughty,           "Naughty"},
        {NatureCheckerValue::Quiet,             "Quiet"},
        {NatureCheckerValue::Quirky,            "Quirky"},
        {NatureCheckerValue::Rash,              "Rash"},
        {NatureCheckerValue::Relaxed,           "Relaxed"},
        {NatureCheckerValue::Sassy,             "Sassy"},
        {NatureCheckerValue::Serious,           "Serious"},
        {NatureCheckerValue::Timid,             "Timid"},
    };
    return data;
}
const std::map<std::pair<int, int>, NatureCheckerValue>& NatureCheckerValue_HELPHINDER_TO_ENUM(){
    static std::map<std::pair<int, int>, NatureCheckerValue> data{
        {{ 0, 2 },      NatureCheckerValue::Adamant},
        {{ -1, -1 },    NatureCheckerValue::Neutral}, //Bashful, Docile, Hardy, Quirky, Serious
        {{ 1, 0 },      NatureCheckerValue::Bold},
        {{ 0, 4 },      NatureCheckerValue::Brave},
        {{ 3, 0 },      NatureCheckerValue::Calm},
        {{ 3, 2 },      NatureCheckerValue::Careful},
        {{ 3, 1 },      NatureCheckerValue::Gentle},
        {{ 4, 1 },      NatureCheckerValue::Hasty},
        {{ 1, 2 },      NatureCheckerValue::Impish},
        {{ 4, 2 },      NatureCheckerValue::Jolly},
        {{ 1, 3 },      NatureCheckerValue::Lax},
        {{ 0, 1 },      NatureCheckerValue::Lonely},
        {{ 2, 1 },      NatureCheckerValue::Mild},
        {{ 2, 0 },      NatureCheckerValue::Modest},
        {{ 4, 3 },      NatureCheckerValue::Naive},
        {{ 0, 3 },      NatureCheckerValue::Naughty},
        {{ 2, 4 },      NatureCheckerValue::Quiet},
        {{ 2, 3 },      NatureCheckerValue::Rash},
        {{ 1, 4 },      NatureCheckerValue::Relaxed},
        {{ 3, 4 },      NatureCheckerValue::Sassy},
        {{ 4, 0 },      NatureCheckerValue::Timid},
    };
    return data;
}

NatureCheckerValue NatureCheckerValue_string_to_enum(const std::string& token){
    auto iter = NatureCheckerValue_TOKEN_TO_ENUM().find(token);
    if (iter == NatureCheckerValue_TOKEN_TO_ENUM().end()){
        return NatureCheckerValue::UnableToDetect;
    }
    return iter->second;
}

NatureCheckerValue NatureCheckerValue_helphinder_to_enum(const std::pair<int,int>& token) {
    auto iter = NatureCheckerValue_HELPHINDER_TO_ENUM().find(token);
    if (iter == NatureCheckerValue_HELPHINDER_TO_ENUM().end()) {
        return NatureCheckerValue::UnableToDetect;
    }
    return iter->second;
}





const EnumDatabase<NatureCheckerValue>& NatureCheckerValue_Database(){
    static EnumDatabase<NatureCheckerValue> database({
        {NatureCheckerValue::Adamant,     "adamant",      "Adamant"},
        {NatureCheckerValue::Bashful,     "bashful",      "Bashful"},
        {NatureCheckerValue::Bold,        "bold",         "Bold"},
        {NatureCheckerValue::Brave,       "brave",        "Brave"},
        {NatureCheckerValue::Calm,        "calm",         "Calm"},
        {NatureCheckerValue::Careful,     "careful",      "Careful"},
        {NatureCheckerValue::Docile,      "docile",       "Docile"},
        {NatureCheckerValue::Gentle,      "gentle",       "Gentle"},
        {NatureCheckerValue::Hardy,       "hardy",        "Hardy"},
        {NatureCheckerValue::Hasty,       "hasty",        "Hasty"},
        {NatureCheckerValue::Impish,      "impish",       "Impish"},
        {NatureCheckerValue::Jolly,       "jolly",        "Jolly"},
        {NatureCheckerValue::Lax,         "lax",          "Lax"},
        {NatureCheckerValue::Lonely,      "lonely",       "Lonely"},
        {NatureCheckerValue::Mild,        "mild",         "Mild"},
        {NatureCheckerValue::Modest,      "modest",       "Modest"},
        {NatureCheckerValue::Naive,       "naive",        "Naive"},
        {NatureCheckerValue::Naughty,     "naughty",      "Naughty"},
        {NatureCheckerValue::Quiet,       "quiet",        "Quiet"},
        {NatureCheckerValue::Quirky,      "quirky",       "Quirky"},
        {NatureCheckerValue::Rash,        "rash",         "Rash"},
        {NatureCheckerValue::Relaxed,     "relaxed",      "Relaxed"},
        {NatureCheckerValue::Sassy,       "sassy",        "Sassy"},
        {NatureCheckerValue::Serious,     "serious",      "Serious"},
        {NatureCheckerValue::Timid,       "timid",        "Timid"},
    });
    return database;
}

const EnumDatabase<NatureCheckerFilter>& NatureCheckerFilter_Database(){
    static EnumDatabase<NatureCheckerFilter> database({
        {NatureCheckerFilter::Any,         "any",          "Any"},
        {NatureCheckerFilter::Adamant,     "adamant",      "Adamant"},
        {NatureCheckerFilter::Bashful,     "bashful",      "Bashful"},
        {NatureCheckerFilter::Bold,        "bold",         "Bold"},
        {NatureCheckerFilter::Brave,       "brave",        "Brave"},
        {NatureCheckerFilter::Calm,        "calm",         "Calm"},
        {NatureCheckerFilter::Careful,     "careful",      "Careful"},
        {NatureCheckerFilter::Docile,      "docile",       "Docile"},
        {NatureCheckerFilter::Gentle,      "gentle",       "Gentle"},
        {NatureCheckerFilter::Hardy,       "hardy",        "Hardy"},
        {NatureCheckerFilter::Hasty,       "hasty",        "Hasty"},
        {NatureCheckerFilter::Impish,      "impish",       "Impish"},
        {NatureCheckerFilter::Jolly,       "jolly",        "Jolly"},
        {NatureCheckerFilter::Lax,         "lax",          "Lax"},
        {NatureCheckerFilter::Lonely,      "lonely",       "Lonely"},
        {NatureCheckerFilter::Mild,        "mild",         "Mild"},
        {NatureCheckerFilter::Modest,      "modest",       "Modest"},
        {NatureCheckerFilter::Naive,       "naive",        "Naive"},
        {NatureCheckerFilter::Naughty,     "naughty",      "Naughty"},
        {NatureCheckerFilter::Quiet,       "quiet",        "Quiet"},
        {NatureCheckerFilter::Quirky,      "quirky",       "Quirky"},
        {NatureCheckerFilter::Rash,        "rash",         "Rash"},
        {NatureCheckerFilter::Relaxed,     "relaxed",      "Relaxed"},
        {NatureCheckerFilter::Sassy,       "sassy",        "Sassy"},
        {NatureCheckerFilter::Serious,     "serious",      "Serious"},
        {NatureCheckerFilter::Timid,       "timid",        "Timid"},
    });
    return database;
}



bool NatureChecker_filter_match(NatureCheckerFilter filter, NatureCheckerValue value){
    switch (filter){
    case NatureCheckerFilter::Any:
        return true;
    case NatureCheckerFilter::Adamant:
        return value == NatureCheckerValue::Adamant;
    case NatureCheckerFilter::Bashful:
        if (value == NatureCheckerValue::Neutral) {
            return true;
        }
        return value == NatureCheckerValue::Bashful;
    case NatureCheckerFilter::Bold:
        return value == NatureCheckerValue::Bold;
    case NatureCheckerFilter::Brave:
        return value == NatureCheckerValue::Brave;
    case NatureCheckerFilter::Calm:
        return value == NatureCheckerValue::Calm;
    case NatureCheckerFilter::Careful:
        return value == NatureCheckerValue::Careful;
    case NatureCheckerFilter::Docile:
        if (value == NatureCheckerValue::Neutral) {
            return true;
        }
        return value == NatureCheckerValue::Docile;
    case NatureCheckerFilter::Gentle:
        return value == NatureCheckerValue::Gentle;
    case NatureCheckerFilter::Hardy:
        if (value == NatureCheckerValue::Neutral) {
            return true;
        }
        return value == NatureCheckerValue::Hardy;
    case NatureCheckerFilter::Hasty:
        return value == NatureCheckerValue::Hasty;
    case NatureCheckerFilter::Impish:
        return value == NatureCheckerValue::Impish;
    case NatureCheckerFilter::Jolly:
        return value == NatureCheckerValue::Jolly;
    case NatureCheckerFilter::Lax:
        return value == NatureCheckerValue::Lax;
    case NatureCheckerFilter::Lonely:
        return value == NatureCheckerValue::Lonely;
    case NatureCheckerFilter::Mild:
        return value == NatureCheckerValue::Mild;
    case NatureCheckerFilter::Modest:
        return value == NatureCheckerValue::Modest;
    case NatureCheckerFilter::Naive:
        return value == NatureCheckerValue::Naive;
    case NatureCheckerFilter::Naughty:
        return value == NatureCheckerValue::Naughty;
    case NatureCheckerFilter::Quiet:
        return value == NatureCheckerValue::Quiet;
    case NatureCheckerFilter::Quirky:
        if (value == NatureCheckerValue::Neutral) {
            return true;
        }
        return value == NatureCheckerValue::Quirky;
    case NatureCheckerFilter::Rash:
        return value == NatureCheckerValue::Rash;
    case NatureCheckerFilter::Relaxed:
        return value == NatureCheckerValue::Relaxed;
    case NatureCheckerFilter::Sassy:
        return value == NatureCheckerValue::Sassy;
    case NatureCheckerFilter::Serious:
        if (value == NatureCheckerValue::Neutral) {
            return true;
        }
        return value == NatureCheckerValue::Serious;
    case NatureCheckerFilter::Timid:
        return value == NatureCheckerValue::Timid;
    }
    return false;
}




}
}
