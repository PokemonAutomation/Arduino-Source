/*  API Options
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_APIOptions_H
#define PokemonAutomation_APIOptions_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"

namespace PokemonAutomation{

class APIOptions : public GroupOption{
public:
    APIOptions();

public:
    StaticTextOption DESCRIPTION;
    BooleanCheckBoxOption ENABLE_API;
    SimpleIntegerOption<uint16_t> HTTP_PORT;
    SimpleIntegerOption<uint16_t> WS_PORT;
};

}

#endif
