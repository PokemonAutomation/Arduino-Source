/*  Controller Mapping: Keyboard -> Procon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_ProController_from_Keyboard_H
#define PokemonAutomation_NintendoSwitch_ProController_from_Keyboard_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "ControllerInput/Keyboard/KeyBindingOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


struct ProControllerDeltas;




class ProControllerFromKeyboardTableRow : public EditableTableRow{
public:
    ProControllerFromKeyboardTableRow(EditableTableOption& parent_table);
    ProControllerFromKeyboardTableRow(
        EditableTableOption& parent_table,
        bool advanced_mode,
        std::string description,
        KeyboardKey key,
        const ProControllerDeltas& deltas
    );
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    ProControllerDeltas snapshot() const;

    void set_advanced_mode(bool enabled);

private:
    std::atomic<bool> m_advanced_mode;

public:
    StringCell label;
    KeyboardHidBindingCell key;
    SimpleIntegerCell<ButtonFlagType> buttons;
    SimpleIntegerCell<int8_t> dpad_x;
    SimpleIntegerCell<int8_t> dpad_y;
    SimpleIntegerCell<int8_t> left_stick_x;
    SimpleIntegerCell<int8_t> left_stick_y;
    SimpleIntegerCell<int8_t> right_stick_x;
    SimpleIntegerCell<int8_t> right_stick_y;
};



class ProControllerFromKeyboardTable : public EditableTableOption_t<ProControllerFromKeyboardTableRow>{
public:
    ProControllerFromKeyboardTable();
    virtual std::vector<std::string> make_header() const override;

    bool advanced_mode() const{ return m_advanced_mode.load(std::memory_order_relaxed); }
    void set_advanced_mode(bool enabled);

    std::unique_ptr<EditableTableRow> make_mapping(
        std::string description,
        KeyboardKey key,
        const ProControllerDeltas& deltas
    );
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

private:
    std::atomic<bool> m_advanced_mode;
};




}
}
#endif
