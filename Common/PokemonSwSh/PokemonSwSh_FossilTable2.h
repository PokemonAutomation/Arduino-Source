/*  Fossil Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FossilTableOption_H
#define PokemonAutomation_PokemonSwSh_FossilTableOption_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class FossilGame2 : public EditableTableRow2{
public:
    FossilGame2()
        : game_slot(
            {
                "Game 1",
                "Game 2",
            },
            0
        )
        , user_slot(
            {
                "User 1",
                "User 2",
                "User 3",
                "User 4",
                "User 5",
                "User 6",
                "User 7",
                "User 8",
            },
            0
        )
        , fossil(
            {
                "Dracozolt",
                "Arctozolt",
                "Dracovish",
                "Arctovish",
            },
            2
        )
        , revives(960, 0, 965)
    {
        PA_ADD_OPTION(game_slot);
        PA_ADD_OPTION(user_slot);
        PA_ADD_OPTION(fossil);
        PA_ADD_OPTION(revives);
    }
    virtual std::unique_ptr<EditableTableRow2> clone() const override{
        std::unique_ptr<FossilGame2> ret(new FossilGame2());
        ret->game_slot.set(game_slot);
        ret->user_slot.set(user_slot);
        ret->fossil.set(fossil);
        ret->revives.set(revives);
        return ret;
    }

public:
    EnumDropdownCell game_slot;
    EnumDropdownCell user_slot;
    EnumDropdownCell fossil;
    SimpleIntegerCell<uint16_t> revives;
};


class FossilTable2Factory : public EditableTableFactory2{
public:
    virtual std::vector<std::string> make_header() const override{
        return std::vector<std::string>{
            "Game",
            "User",
            "Fossil",
            "Revives"
        };
    }
    virtual std::unique_ptr<EditableTableRow2> make_row() const override{
        return std::unique_ptr<EditableTableRow2>(new FossilGame2());
    }
};




}
}
}
#endif
