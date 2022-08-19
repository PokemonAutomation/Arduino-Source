/*  Pokemon IV Checker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
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


const EnumDatabase<IVCheckerValue>& IVCheckerValue_Database(){
    static EnumDatabase<IVCheckerValue> database({
        {IVCheckerValue::NoGood,       "no-good",       "No Good (0)"},
        {IVCheckerValue::Decent,       "decent",        "Decent (1-15)"},
        {IVCheckerValue::PrettyGood,   "pretty-good",   "Pretty Good (16-25)"},
        {IVCheckerValue::VeryGood,     "very-good",     "Very Good (26-29)"},
        {IVCheckerValue::Fantastic,    "fantastic",     "Fantastic (30)"},
        {IVCheckerValue::Best,         "best",          "Best (31)"},
        {IVCheckerValue::HyperTrained, "hyper-trained", "Hyper trained!"},
    });
    return database;
}






const EnumDatabase<IVCheckerFilter>& IVCheckerFilter_Database(){
    static EnumDatabase<IVCheckerFilter> database({
        {IVCheckerFilter::Anything,     "anything",     "Anything (0-31)"},
        {IVCheckerFilter::NoGood,       "no-good",      "No Good (0)"},
        {IVCheckerFilter::Decent,       "decent",       "Decent (1-15)"},
        {IVCheckerFilter::PrettyGood,   "pretty-good",  "Pretty Good (16-25)"},
        {IVCheckerFilter::VeryGood,     "very-good",    "Very Good (26-29)"},
        {IVCheckerFilter::Fantastic,    "fantastic",    "Fantastic (30)"},
        {IVCheckerFilter::Best,         "best",         "Best (31)"},
    });
    return database;
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
