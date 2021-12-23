/*  Pokemon IV Checker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/Exception.h"
#include "Pokemon_IVChecker.h"

namespace PokemonAutomation{
namespace Pokemon{


const std::map<std::string, IVCheckerValue> IVCheckerValue_TOKEN_TO_ENUM{
    {"No Good",         IVCheckerValue::NoGood},
    {"Decent",          IVCheckerValue::Decent},
    {"Pretty Good",     IVCheckerValue::PrettyGood},
    {"Very Good",       IVCheckerValue::VeryGood},
    {"Fantastic",       IVCheckerValue::Fantastic},
    {"Best",            IVCheckerValue::Best},
    {"Hyper trained!",  IVCheckerValue::HyperTrained},
};
const std::map<IVCheckerValue, std::string> IVCheckerValue_ENUM_TO_TOKEN{
    {IVCheckerValue::UnableToDetect,    "Unable to Detect"},
    {IVCheckerValue::NoGood,            "No Good"},
    {IVCheckerValue::Decent,            "Decent"},
    {IVCheckerValue::PrettyGood,        "Pretty Good"},
    {IVCheckerValue::VeryGood,          "Very Good"},
    {IVCheckerValue::Fantastic,         "Fantastic"},
    {IVCheckerValue::Best,              "Best"},
    {IVCheckerValue::HyperTrained,      "Hyper trained!"},
};
IVCheckerValue IVCheckerValue_string_to_enum(const std::string& token){
    auto iter = IVCheckerValue_TOKEN_TO_ENUM.find(token);
    if (iter == IVCheckerValue_TOKEN_TO_ENUM.end()){
        return IVCheckerValue::UnableToDetect;
    }
    return iter->second;
}
const std::string& IVCheckerValue_enum_to_string(IVCheckerValue result){
    auto iter = IVCheckerValue_ENUM_TO_TOKEN.find(result);
    if (iter == IVCheckerValue_ENUM_TO_TOKEN.end()){
        PA_THROW_StringException("Invalid IV result enum.");
    }
    return iter->second;
}


const std::vector<QString> IVCheckerFilter_NAMES{
    "Anything (0-31)",
    "No Good (0)",
    "Decent (0-15)",
    "Pretty Good (16-25)",
    "Very Good (26-29)",
    "Fantastic (30)",
    "Best (31)",
};
const std::map<QString, IVCheckerFilter> IVCheckerFilter_TOKEN_TO_ENUM{
    {IVCheckerFilter_NAMES[0],  IVCheckerFilter::Anything},
    {IVCheckerFilter_NAMES[1],  IVCheckerFilter::NoGood},
    {IVCheckerFilter_NAMES[2],  IVCheckerFilter::Decent},
    {IVCheckerFilter_NAMES[3],  IVCheckerFilter::PrettyGood},
    {IVCheckerFilter_NAMES[4],  IVCheckerFilter::VeryGood},
    {IVCheckerFilter_NAMES[5],  IVCheckerFilter::Fantastic},
    {IVCheckerFilter_NAMES[6],  IVCheckerFilter::Best},
};
IVCheckerFilter IVCheckerFilter_string_to_enum(const QString& token){
    auto iter = IVCheckerFilter_TOKEN_TO_ENUM.find(token);
    if (iter == IVCheckerFilter_TOKEN_TO_ENUM.end()){
        return IVCheckerFilter::Anything;
    }
    return iter->second;
}
const QString& IVCheckerFilter_enum_to_string(IVCheckerFilter result){
    return IVCheckerFilter_NAMES[(size_t)result];
}



bool IVChecker_filter_match(IVCheckerFilter filter, IVCheckerValue value){
    switch (filter){
    case IVCheckerFilter::Anything:
        return true;
    case IVCheckerFilter::NoGood:
        return value == IVCheckerValue::NoGood;
    case IVCheckerFilter::Decent:
        return value == IVCheckerValue::Decent;
    case IVCheckerFilter::PrettyGood:
        return value == IVCheckerValue::PrettyGood;
    case IVCheckerFilter::VeryGood:
        return value == IVCheckerValue::VeryGood;
    case IVCheckerFilter::Fantastic:
        return value == IVCheckerValue::Fantastic;
    case IVCheckerFilter::Best:
        return value == IVCheckerValue::Best;
    }
    return false;
}




}
}
