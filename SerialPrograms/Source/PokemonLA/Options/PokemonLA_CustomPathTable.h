/*  Custom Path Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_CustomPathTableTable_H
#define PokemonAutomation_PokemonLA_CustomPathTableTable_H

#include "Common/Cpp/Options/ConfigOption.h"
#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "PokemonLA_TravelLocation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



enum class PathAction{
    NO_ACTION,
    CHANGE_MOUNT,
    MOVE_FORWARD,
    MOVE_IN_DIRECTION,
    CENTER_CAMERA,
    JUMP,
    WAIT,
    START_LISTEN,
    END_LISTEN,
};
const EnumDropdownDatabase<PathAction>& PathAction_Database();

enum class PathMount{
    NO_MOUNT,
    WYRDEER,
    URSALUNA,
    BASCULEGION,
    SNEASLER,
    BRAVIARY,
};
const EnumDropdownDatabase<PathMount>& PathMount_Database();

enum class PathSpeed{
    NORMAL_SPEED,
    SLOW_SPEED,
    RUN,
    DASH,
    DASH_B_SPAM,
    DIVE,
};
const EnumDropdownDatabase<PathSpeed>& PathSpeed_Database();




class CustomPathCell : public BatchOption, private ConfigOption::Listener{
public:
    ~CustomPathCell();
    void operator=(const CustomPathCell& x);
    CustomPathCell(EnumDropdownCell<PathAction>& action);

    virtual void on_config_value_changed(void* object) override;

private:
    EnumDropdownCell<PathAction>& m_action;

public:
    StaticTextOption text;
    EnumDropdownCell<PathMount> mount;
    MillisecondsOption move_forward;
    EnumDropdownCell<PathSpeed> move_speed;
    FloatingPointOption left_x;
    FloatingPointOption left_y;
    MillisecondsOption jump_wait;
    MillisecondsOption wait;
};

class CustomPathTableRow : public EditableTableRow{
public:
    CustomPathTableRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

    virtual void load_json(const JsonValue& json) override;
    virtual JsonValue to_json() const override;

public:
    EnumDropdownCell<PathAction> action;
    CustomPathCell parameters;
};

class CustomPathTable : public EditableTableOption_t<CustomPathTableRow>{
public:
    CustomPathTable();
    virtual std::vector<std::string> make_header() const override;

private:
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};





// A program option to build a custom path to navigate the map
class CustomPathTableFromJubilife : public BatchOption{
public:
    CustomPathTableFromJubilife();

    const WildTravelLocationOption& travel_location() const{ return TRAVEL_LOCATION; }

    virtual ConfigWidget* make_QtWidget(QWidget& parent) override;

public:
    friend class CustomPathTableWidget;

    WildTravelLocationOption TRAVEL_LOCATION;
    CustomPathTable PATH;
};





}
}
}
#endif
