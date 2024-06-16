/*  Encounter Filter Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_EncounterFilterTable_H
#define PokemonAutomation_PokemonSV_EncounterFilterTable_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "CommonFramework/Options/StringSelectOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


enum class EncounterActionsAction{
    STOP_PROGRAM,
    RUN_AWAY,
    THROW_BALLS,
    THROW_BALLS_AND_SAVE,
};

enum class EncounterActionsShininess{
    ANYTHING,
    NOT_SHINY,
    SHINY,
};



struct EncounterActionsEntry{
    EncounterActionsAction action;
    std::string ball;
    std::string pokemon;
    EncounterActionsShininess shininess;
};


class EncounterActionsRow : public EditableTableRow, private ConfigOption::Listener{
public:
    ~EncounterActionsRow();
    EncounterActionsRow();

    virtual std::unique_ptr<EditableTableRow> clone() const override;

    EncounterActionsEntry snapshot() const;

private:
    virtual void value_changed(void* object) override;

private:
    EnumDropdownCell<EncounterActionsAction> action;
    PokemonSwSh::PokemonBallSelectCell pokeball;
    StringSelectCell pokemon;
    EnumDropdownCell<EncounterActionsShininess> shininess;
};


class EncounterActionsTable : public EditableTableOption_t<EncounterActionsRow>{
public:
    EncounterActionsTable();

    std::vector<EncounterActionsEntry> snapshot();

    virtual std::vector<std::string> make_header() const;

    static std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

};






}
}
}
#endif
