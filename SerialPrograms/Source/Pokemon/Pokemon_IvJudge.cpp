/*  Pokemon IV Checker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon_IvJudge.h"

namespace PokemonAutomation{
namespace Pokemon{



const EnumStringMap<IvJudgeValue>& IV_JUDGE_VALUE_STRINGS(){
    static EnumStringMap<IvJudgeValue> database{
        {IvJudgeValue::UnableToDetect,    "Unable to Detect"},
        {IvJudgeValue::NoGood,            "No Good"},
        {IvJudgeValue::Decent,            "Decent"},
        {IvJudgeValue::PrettyGood,        "Pretty Good"},
        {IvJudgeValue::VeryGood,          "Very Good"},
        {IvJudgeValue::Fantastic,         "Fantastic"},
        {IvJudgeValue::Best,              "Best"},
        {IvJudgeValue::HyperTrained,      "Hyper trained!"},
    };
    return database;
}
const EnumDropdownDatabase<IvJudgeValue>& IvJudgeValue_Database(){
    static EnumDropdownDatabase<IvJudgeValue> database({
        {IvJudgeValue::NoGood,       "no-good",       "No Good (0)"},
        {IvJudgeValue::Decent,       "decent",        "Decent (1-15)"},
        {IvJudgeValue::PrettyGood,   "pretty-good",   "Pretty Good (16-25)"},
        {IvJudgeValue::VeryGood,     "very-good",     "Very Good (26-29)"},
        {IvJudgeValue::Fantastic,    "fantastic",     "Fantastic (30)"},
        {IvJudgeValue::Best,         "best",          "Best (31)"},
        {IvJudgeValue::HyperTrained, "hyper-trained", "Hyper trained!"},
    });
    return database;
}






const EnumDropdownDatabase<IvJudgeFilter>& IvJudgeFilter_Database(){
    static EnumDropdownDatabase<IvJudgeFilter> database({
        {IvJudgeFilter::Anything,     "anything",     "Anything (0-31)"},
        {IvJudgeFilter::NoGood,       "no-good",      "No Good (0)"},
        {IvJudgeFilter::Decent,       "decent",       "Decent (1-15)"},
        {IvJudgeFilter::PrettyGood,   "pretty-good",  "Pretty Good (16-25)"},
        {IvJudgeFilter::VeryGood,     "very-good",    "Very Good (26-29)"},
        {IvJudgeFilter::Fantastic,    "fantastic",    "Fantastic (30)"},
        {IvJudgeFilter::Best,         "best",         "Best (31)"},
    });
    return database;
}



bool IvJudge_filter_match(IvJudgeFilter filter, IvJudgeValue value){
    switch (filter){
    case IvJudgeFilter::Anything:
        return true;
    case IvJudgeFilter::NoGood:
        return value == IvJudgeValue::NoGood;
    case IvJudgeFilter::Decent:
        return value == IvJudgeValue::Decent;
    case IvJudgeFilter::PrettyGood:
        return value == IvJudgeValue::PrettyGood;
    case IvJudgeFilter::VeryGood:
        return value == IvJudgeValue::VeryGood;
    case IvJudgeFilter::Fantastic:
        return value == IvJudgeValue::Fantastic;
    case IvJudgeFilter::Best:
        return value == IvJudgeValue::Best;
    }
    return false;
}




}
}
