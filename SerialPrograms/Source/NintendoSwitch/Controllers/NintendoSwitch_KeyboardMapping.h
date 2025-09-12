/*  Nintendo Keyboard Mapping
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_KeyboardMapping_H
#define PokemonAutomation_NintendoSwitch_KeyboardMapping_H

#include <Qt>
#include "Common/Cpp/Options/BatchOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/KeyBindingOption.h"
#include "NintendoSwitch_ControllerButtons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


struct ProControllerDeltas;
struct JoyconDeltas;



class ProControllerKeyMapTableRow : public EditableTableRow{
public:
    ProControllerKeyMapTableRow(EditableTableOption& parent_table);
    ProControllerKeyMapTableRow(
        EditableTableOption& parent_table,
        bool advanced_mode,
        std::string description,
        Qt::Key key,
        const ProControllerDeltas& deltas
    );
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    ProControllerDeltas snapshot() const;

    void set_advanced_mode(bool enabled);

private:
    std::atomic<bool> m_advanced_mode;

public:
    StringCell label;
    KeyBindingCell key;
    SimpleIntegerCell<ButtonFlagType> buttons;
    SimpleIntegerCell<int8_t> dpad_x;
    SimpleIntegerCell<int8_t> dpad_y;
    SimpleIntegerCell<int8_t> left_stick_x;
    SimpleIntegerCell<int8_t> left_stick_y;
    SimpleIntegerCell<int8_t> right_stick_x;
    SimpleIntegerCell<int8_t> right_stick_y;
};

class ProControllerKeyboardMappingTable : public EditableTableOption_t<ProControllerKeyMapTableRow>{
public:
    ProControllerKeyboardMappingTable();
    virtual std::vector<std::string> make_header() const override;

    bool advanced_mode() const{ return m_advanced_mode.load(std::memory_order_relaxed); }
    void set_advanced_mode(bool enabled);

    std::unique_ptr<EditableTableRow> make_mapping(
        std::string description,
        Qt::Key key,
        const ProControllerDeltas& deltas
    );
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

private:
    std::atomic<bool> m_advanced_mode;
};




class JoyconKeyMapTableRow : public EditableTableRow{
public:
    JoyconKeyMapTableRow(EditableTableOption& parent_table);
    JoyconKeyMapTableRow(
        EditableTableOption& parent_table,
        bool advanced_mode,
        std::string description,
        Qt::Key key,
        const JoyconDeltas& deltas
    );
    virtual std::unique_ptr<EditableTableRow> clone() const override;
    JoyconDeltas snapshot() const;

    void set_advanced_mode(bool enabled);

private:
    std::atomic<bool> m_advanced_mode;

public:
    StringCell label;
    KeyBindingCell key;
    SimpleIntegerCell<ButtonFlagType> buttons;
    SimpleIntegerCell<int8_t> joystick_x;
    SimpleIntegerCell<int8_t> joystick_y;
};

class JoyconKeyboardMappingTable : public EditableTableOption_t<JoyconKeyMapTableRow>{
public:
    JoyconKeyboardMappingTable(bool left);
    virtual std::vector<std::string> make_header() const override;

    bool advanced_mode() const{ return m_advanced_mode.load(std::memory_order_relaxed); }
    void set_advanced_mode(bool enabled);

    std::unique_ptr<EditableTableRow> make_mapping(
        std::string description,
        Qt::Key key,
        const JoyconDeltas& deltas
    );
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults(bool left);

private:
    std::atomic<bool> m_advanced_mode;
};








class KeyboardMappingOption : public BatchOption, private ConfigOption::Listener{
public:
    ~KeyboardMappingOption();
    KeyboardMappingOption();

private:
    virtual void load_json(const JsonValue& json) override;
    virtual void on_config_value_changed(void* object) override;

public:
    StaticTextOption DESCRIPTION;
    BooleanCheckBoxOption ADVANCED_MODE;
    ProControllerKeyboardMappingTable PRO_CONTROLLER1;
    JoyconKeyboardMappingTable LEFT_JOYCON0;
    JoyconKeyboardMappingTable RIGHT_JOYCON0;
};



}
}
#endif
