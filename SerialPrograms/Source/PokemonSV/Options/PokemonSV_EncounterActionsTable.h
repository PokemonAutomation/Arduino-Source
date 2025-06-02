/*  Encounter Filter Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_EncounterFilterTable_H
#define PokemonAutomation_PokemonSV_EncounterFilterTable_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "CommonTools/Options/StringSelectOption.h"
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
    std::string pokemon;
    EncounterActionsShininess shininess;
    EncounterActionsAction action;
    std::string ball;
    uint16_t ball_limit;
};


class EncounterActionsRow : public EditableTableRow, private ConfigOption::Listener{
public:
    ~EncounterActionsRow();
    EncounterActionsRow(EditableTableOption& parent_table);

    virtual std::unique_ptr<EditableTableRow> clone() const override;

    EncounterActionsEntry snapshot() const;

private:
    virtual void on_config_value_changed(void* object) override;

private:
    StringSelectCell pokemon;
    EnumDropdownCell<EncounterActionsShininess> shininess;
    EnumDropdownCell<EncounterActionsAction> action;
    PokemonSwSh::PokemonBallSelectCell pokeball;
    SimpleIntegerCell<uint16_t> ball_limit;
};


class EncounterActionsTable : public EditableTableOption_t<EncounterActionsRow>{
public:
    EncounterActionsTable();

    std::vector<EncounterActionsEntry> snapshot();

    virtual std::vector<std::string> make_header() const;

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

};






}
}
}
#endif
