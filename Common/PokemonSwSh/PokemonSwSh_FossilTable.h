/*  Fossil Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FossilTableOption_H
#define PokemonAutomation_PokemonSwSh_FossilTableOption_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/DropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class FossilGame : public EditableTableRow2{
public:
    enum Fossil{
        Dracozolt   =   0,
        Arctozolt   =   1,
        Dracovish   =   2,
        Arctovish   =   3,
    };

public:
    FossilGame()
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
    FossilGame(uint8_t p_game_slot, uint8_t p_user_slot, Fossil p_fossil, uint16_t p_revives)
        : FossilGame()
    {
        game_slot.set_index(p_game_slot - 1);
        user_slot.set_index(p_user_slot - 1);
        fossil.set_index((size_t)p_fossil);
        revives.set(p_revives);
    }
    virtual std::unique_ptr<EditableTableRow2> clone() const override{
        std::unique_ptr<FossilGame> ret(new FossilGame());
        ret->game_slot.set_index(game_slot);
        ret->user_slot.set_index(user_slot);
        ret->fossil.set_index(fossil);
        ret->revives.set(revives);
        return ret;
    }

public:
    DropdownCell game_slot;
    DropdownCell user_slot;
    DropdownCell fossil;
    SimpleIntegerCell<uint16_t> revives;
};


class FossilTable : public EditableTableOption_t<FossilGame>{
public:
    FossilTable()
        : EditableTableOption_t<FossilGame>("<b>Game List:</b>", make_defaults())
    {}
    virtual std::vector<std::string> make_header() const override{
        return std::vector<std::string>{
            "Game",
            "User",
            "Fossil",
            "Revives"
        };
    }

    static std::vector<std::unique_ptr<EditableTableRow2>> make_defaults(){
        std::vector<std::unique_ptr<EditableTableRow2>> ret;
        ret.emplace_back(new FossilGame(1, 1, FossilGame::Dracovish, 960));
        return ret;
    }
};




}
}
}
#endif
