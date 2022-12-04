/*  Fossil Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_FossilTableOption_H
#define PokemonAutomation_PokemonSwSh_FossilTableOption_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/NintendoSwitch/NintendoSwitch_SlotDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


enum class Fossil{
    Dracozolt   =   0,
    Arctozolt   =   1,
    Dracovish   =   2,
    Arctovish   =   3,
};


class FossilGame : public EditableTableRow{
private:
    static const EnumDatabase<Fossil>& Fossil_Database(){
        static const EnumDatabase<Fossil> database({
            {Fossil::Dracozolt, "dracozolt", "Dracozolt"},
            {Fossil::Arctozolt, "arctozolt", "Arctozolt"},
            {Fossil::Dracovish, "dracovish", "Dracovish"},
            {Fossil::Arctovish, "arctovish", "Arctovish"},
        });
        return database;
    }

public:
    FossilGame()
        : game_slot(GameSlot_Database(), LockWhileRunning::LOCKED, 1)
        , user_slot(UserSlot_Database(), LockWhileRunning::LOCKED, 1)
        , fossil(Fossil_Database(), LockWhileRunning::LOCKED, Fossil::Dracovish)
        , revives(LockWhileRunning::LOCKED, 960, 0, 965)
    {
        PA_ADD_OPTION(game_slot);
        PA_ADD_OPTION(user_slot);
        PA_ADD_OPTION(fossil);
        PA_ADD_OPTION(revives);
    }
    FossilGame(uint8_t p_game_slot, uint8_t p_user_slot, Fossil p_fossil, uint16_t p_revives)
        : FossilGame()
    {
        game_slot.set_value(p_game_slot);
        user_slot.set_value(p_user_slot);
        fossil.set(p_fossil);
        revives.set(p_revives);
    }
    virtual std::unique_ptr<EditableTableRow> clone() const override{
        std::unique_ptr<FossilGame> ret(new FossilGame());
        ret->game_slot.set_value(game_slot.current_value());
        ret->user_slot.set_value(user_slot.current_value());
        ret->fossil.set(fossil);
        ret->revives.set(revives);
        return ret;
    }

public:
    IntegerEnumDropdownCell game_slot;
    IntegerEnumDropdownCell user_slot;
    EnumDropdownCell<Fossil> fossil;
    SimpleIntegerCell<uint16_t> revives;
};


class FossilTable : public EditableTableOption_t<FossilGame>{
public:
    FossilTable()
        : EditableTableOption_t<FossilGame>(
            "<b>Game List:</b>",
            LockWhileRunning::LOCKED,
            make_defaults()
        )
    {}
    virtual std::vector<std::string> make_header() const override{
        return std::vector<std::string>{
            "Game",
            "User",
            "Fossil",
            "Revives"
        };
    }

    static std::vector<std::unique_ptr<EditableTableRow>> make_defaults(){
        std::vector<std::unique_ptr<EditableTableRow>> ret;
        ret.emplace_back(new FossilGame(1, 1, Fossil::Dracovish, 960));
        return ret;
    }
};




}
}
}
#endif
