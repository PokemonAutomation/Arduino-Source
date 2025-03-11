/*  Fossil Table
 *
 *  From: https://github.com/PokemonAutomation/
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
    static const EnumDropdownDatabase<Fossil>& Fossil_Database(){
        static const EnumDropdownDatabase<Fossil> database({
            {Fossil::Dracozolt, "dracozolt", "Dracozolt"},
            {Fossil::Arctozolt, "arctozolt", "Arctozolt"},
            {Fossil::Dracovish, "dracovish", "Dracovish"},
            {Fossil::Arctovish, "arctovish", "Arctovish"},
        });
        return database;
    }

public:
    FossilGame(EditableTableOption& parent_table)
        : EditableTableRow(parent_table)
        , game_slot(GameSlot_Database(), LockMode::LOCK_WHILE_RUNNING, 1)
        , user_slot(UserSlot_Database(), LockMode::LOCK_WHILE_RUNNING, 1)
        , fossil(Fossil_Database(), LockMode::LOCK_WHILE_RUNNING, Fossil::Dracovish)
        , revives(LockMode::LOCK_WHILE_RUNNING, 960, 0, 965)
    {
        PA_ADD_OPTION(game_slot);
        PA_ADD_OPTION(user_slot);
        PA_ADD_OPTION(fossil);
        PA_ADD_OPTION(revives);
    }
    FossilGame(
        EditableTableOption& parent_table,
        uint8_t p_game_slot, uint8_t p_user_slot, Fossil p_fossil, uint16_t p_revives
    )
        : FossilGame(parent_table)
    {
        game_slot.set_value(p_game_slot);
        user_slot.set_value(p_user_slot);
        fossil.set(p_fossil);
        revives.set(p_revives);
    }
    virtual std::unique_ptr<EditableTableRow> clone() const override{
        std::unique_ptr<FossilGame> ret(new FossilGame(parent()));
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
            LockMode::LOCK_WHILE_RUNNING,
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

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults(){
        std::vector<std::unique_ptr<EditableTableRow>> ret;
        ret.emplace_back(new FossilGame(*this, 1, 1, Fossil::Dracovish, 960));
        return ret;
    }
};




}
}
}
#endif
