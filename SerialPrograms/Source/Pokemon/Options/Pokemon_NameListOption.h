/*  Pokemon Name List Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameListOption_H
#define PokemonAutomation_Pokemon_PokemonNameListOption_H

#include "CommonFramework/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace Pokemon{


class PokemonNameList : public EditableTableOption{
public:
    PokemonNameList(QString label);

    const std::string& operator[](size_t index) const;
};


}
}
#endif
