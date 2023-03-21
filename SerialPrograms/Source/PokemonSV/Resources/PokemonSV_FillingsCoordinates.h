/*  Fillings Coordinates
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_FillingsCoordinates_H
#define PokemonAutomation_PokemonSV_FillingsCoordinates_H

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSV {

class FillingsCoordinates {
    struct FillingInfo {
        uint8_t piecesPerServing;
        uint8_t servingsPerBowl;
    };

    const std::map<std::string, FillingInfo> SandwichFillingsData{
        {"baguette",            {0, 0}},
        {"lettuce",             {3, 2}},
        {"tomato",              {3, 2}},
        {"cherry-tomatoes",     {3, 3}},
        {"cucumber",            {3, 2}},
        {"pickle",              {3, 2}},
        {"onion",               {3, 2}},
        {"red-onion",           {3, 2}},
        {"green-bell-pepper",   {3, 2}},
        {"red-bell-pepper",     {3, 2}},
        {"yellow-bell-pepper",  {3, 2}},
        {"avocado",             {3, 2}},
        {"bacon",               {3, 2}},
        {"ham",                 {3, 2}},
        {"prosciutto",          {3, 2}},
        {"chorizo",             {3, 2}},
        {"herbed-sausage",      {3, 2}},
        {"hamburger",           {1, 1}},
        {"klawf-stick",         {3, 1}},
        {"smoked-fillet",       {3, 2}},
        {"fried-fillet",        {1, 1}},
        {"egg",                 {3, 1}},
        {"potato-tortilla",     {1, 1}},
        {"tofu",                {3, 1}},
        {"rice",                {1, 1}},
        {"noodles",             {1, 1}},
        {"potato-salad",        {1, 1}},
        {"cheese",              {3, 2}},
        {"banana",              {3, 2}},
        {"strawberry",          {3, 2}},
        {"apple",               {3, 2}},
        {"kiwi",                {3, 2}},
        {"pineapple",           {3, 2}},
        {"jalape\xF1o",         {3, 2}},
        {"watercress",          {3, 2}},
        {"basil",               {4, 2}},
    };

public:
    FillingInfo get_filling_information(std::string filling) {
        auto iter = SandwichFillingsData.find(filling);
        return iter->second;
    }


};
}
}
}
#endif
