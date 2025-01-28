/*  Nintendo Keyboard Mapping
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_KeyboardMapping_H
#define PokemonAutomation_NintendoSwitch_KeyboardMapping_H

#include <Qt>
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/KeyBindingOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


struct ControllerDeltas;



class KeyMapTableRow : public EditableTableRow{
public:
    ~KeyMapTableRow();
    KeyMapTableRow(EditableTableOption& parent_table);
    KeyMapTableRow(
        EditableTableOption& parent_table,
        std::string description,
        Qt::Key key,
        const ControllerDeltas& deltas
    );
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    ControllerDeltas snapshot() const;

//    virtual void value_changed(void* object) override;

public:
    StringCell label;
//    SimpleIntegerCell<uint32_t> qt_key;
//    StringCell key;
    KeyBindingCell key;
    SimpleIntegerCell<uint16_t> buttons;
    SimpleIntegerCell<int8_t> dpad_x;
    SimpleIntegerCell<int8_t> dpad_y;
    SimpleIntegerCell<int8_t> left_stick_x;
    SimpleIntegerCell<int8_t> left_stick_y;
    SimpleIntegerCell<int8_t> right_stick_x;
    SimpleIntegerCell<int8_t> right_stick_y;
};

class KeyboardMappingTable : public EditableTableOption_t<KeyMapTableRow>{
public:
    KeyboardMappingTable();
    virtual std::vector<std::string> make_header() const override;

    std::unique_ptr<EditableTableRow> make_mapping(
        std::string description,
        Qt::Key key,
        const ControllerDeltas& deltas
    );
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};




}
}
#endif
