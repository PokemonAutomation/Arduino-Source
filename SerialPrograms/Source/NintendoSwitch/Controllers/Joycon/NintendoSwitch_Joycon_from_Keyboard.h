/*  Controller Mapping: Keyboard -> Joycon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Joycon_from_Keyboard_H
#define PokemonAutomation_NintendoSwitch_Joycon_from_Keyboard_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "ControllerInput/Keyboard/KeyBindingOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


struct JoyconDeltas;




class JoyconFromKeyboardTableRow : public EditableTableRow{
public:
    JoyconFromKeyboardTableRow(EditableTableOption& parent_table);
    JoyconFromKeyboardTableRow(
        EditableTableOption& parent_table,
        bool advanced_mode,
        std::string description,
        KeyboardKey key,
        const JoyconDeltas& deltas
    );
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    JoyconDeltas snapshot() const;

    void set_advanced_mode(bool enabled);

private:
    std::atomic<bool> m_advanced_mode;

public:
    StringCell label;
    KeyboardHidBindingCell key;
    SimpleIntegerCell<ButtonFlagType> buttons;
    SimpleIntegerCell<int8_t> joystick_x;
    SimpleIntegerCell<int8_t> joystick_y;
};



class JoyconFromKeyboardTable : public EditableTableOption_t<JoyconFromKeyboardTableRow>{
public:
    JoyconFromKeyboardTable(bool left);
    virtual std::vector<std::string> make_header() const override;

    bool advanced_mode() const{ return m_advanced_mode.load(std::memory_order_relaxed); }
    void set_advanced_mode(bool enabled);

    std::unique_ptr<EditableTableRow> make_mapping(
        std::string description,
        KeyboardKey key,
        const JoyconDeltas& deltas
    );
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults(bool left);

private:
    std::atomic<bool> m_advanced_mode;
};




}
}
#endif
