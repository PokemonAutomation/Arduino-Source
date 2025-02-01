/*  Nintendo Keyboard Mapping
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QKeySequence>
#include "NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch_KeyboardMapping.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


KeyMapTableRow::KeyMapTableRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , label(false, LockMode::UNLOCK_WHILE_RUNNING, "", "")
    , key(LockMode::UNLOCK_WHILE_RUNNING)
    , buttons(LockMode::UNLOCK_WHILE_RUNNING, 0, 0, ((uint16_t)1 << 14) - 1)
    , dpad_x(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
    , dpad_y(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
    , left_stick_x(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
    , left_stick_y(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
    , right_stick_x(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
    , right_stick_y(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
{
    add_option(label, "Description");
    add_option(key, "Key");
    add_option(buttons, "Button Bit-Field");
    add_option(dpad_x, "Dpad x");
    add_option(dpad_y, "Dpad y");
    add_option(left_stick_x, "Left-Stick x");
    add_option(left_stick_y, "Left-Stick y");
    add_option(right_stick_x, "Right-Stick x");
    add_option(right_stick_y, "Right-Stick y");
    set_advanced_mode(static_cast<KeyboardMappingTable&>(parent_table).advanced_mode());
}
KeyMapTableRow::KeyMapTableRow(
    EditableTableOption& parent_table,
    bool advanced_mode,
    std::string description,
    Qt::Key key,
    const ControllerDeltas& deltas
)
    : KeyMapTableRow(parent_table)
{
    label.set(std::move(description));
    this->key.set(key);
    buttons.set(deltas.buttons);
    dpad_x.set(deltas.dpad_x);
    dpad_y.set(deltas.dpad_y);
    left_stick_x.set(deltas.left_x);
    left_stick_y.set(deltas.left_y);
    right_stick_x.set(deltas.right_x);
    right_stick_y.set(deltas.right_y);
    set_advanced_mode(advanced_mode);
}
std::unique_ptr<EditableTableRow> KeyMapTableRow::clone() const{
    std::unique_ptr<KeyMapTableRow> ret(new KeyMapTableRow(parent()));
    ret->label.set(label);
    ret->key.set((uint32_t)key);
    ret->buttons.set(buttons);
    ret->dpad_x.set(dpad_x);
    ret->dpad_y.set(dpad_y);
    ret->left_stick_x.set(left_stick_x);
    ret->left_stick_y.set(left_stick_y);
    ret->right_stick_x.set(right_stick_x);
    ret->right_stick_y.set(right_stick_y);
    ret->set_advanced_mode(m_advanced_mode.load(std::memory_order_relaxed));
    return ret;
}
ControllerDeltas KeyMapTableRow::snapshot() const{
    return {
        .buttons = (Button)buttons.current_value(),
        .dpad_x = dpad_x,
        .dpad_y = dpad_y,
        .left_x = left_stick_x,
        .left_y = left_stick_y,
        .right_x = right_stick_x,
        .right_y = right_stick_y,
    };
}
void KeyMapTableRow::set_advanced_mode(bool enabled){
    m_advanced_mode.store(enabled, std::memory_order_relaxed);
    if (enabled){
        label.set_locked(false);
        buttons.set_visibility(ConfigOptionState::ENABLED);
        dpad_x.set_visibility(ConfigOptionState::ENABLED);
        dpad_y.set_visibility(ConfigOptionState::ENABLED);
        left_stick_x.set_visibility(ConfigOptionState::ENABLED);
        left_stick_y.set_visibility(ConfigOptionState::ENABLED);
        right_stick_x.set_visibility(ConfigOptionState::ENABLED);
        right_stick_y.set_visibility(ConfigOptionState::ENABLED);
    }else{
        label.set_locked(true);
        buttons.set_visibility(ConfigOptionState::DISABLED);
        dpad_x.set_visibility(ConfigOptionState::DISABLED);
        dpad_y.set_visibility(ConfigOptionState::DISABLED);
        left_stick_x.set_visibility(ConfigOptionState::DISABLED);
        left_stick_y.set_visibility(ConfigOptionState::DISABLED);
        right_stick_x.set_visibility(ConfigOptionState::DISABLED);
        right_stick_y.set_visibility(ConfigOptionState::DISABLED);
    }
}





KeyboardMappingTable::KeyboardMappingTable()
    : EditableTableOption_t<KeyMapTableRow>(
        "",
        LockMode::UNLOCK_WHILE_RUNNING,
        true,
        make_defaults()
    )
    , m_advanced_mode(false)
{}
std::vector<std::string> KeyboardMappingTable::make_header() const{
    return std::vector<std::string>{
        "Description",
        "Key",
        "Button Bit-Field",
        "Dpad x",
        "Dpad y",
        "Left-Stick x",
        "Left-Stick y",
        "Right-Stick x",
        "Right-Stick y",
    };
}
void KeyboardMappingTable::set_advanced_mode(bool enabled){
    m_advanced_mode.store(enabled, std::memory_order_relaxed);
    run_on_all_rows([enabled](KeyMapTableRow& row){
        row.set_advanced_mode(enabled);
    });
}


std::unique_ptr<EditableTableRow> KeyboardMappingTable::make_mapping(
    std::string description,
    Qt::Key key,
    const ControllerDeltas& deltas
){
    return std::make_unique<KeyMapTableRow>(
        *this,
        m_advanced_mode.load(std::memory_order_relaxed),
        std::move(description),
        key,
        deltas
    );
}
std::vector<std::unique_ptr<EditableTableRow>> KeyboardMappingTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;

    ret.emplace_back(make_mapping("Dpad Up",            Qt::Key::Key_8,         ControllerDeltas{.dpad_x =  0, .dpad_y = -1}));
    ret.emplace_back(make_mapping("Dpad Up+Right",      Qt::Key::Key_9,         ControllerDeltas{.dpad_x = +1, .dpad_y = -1}));
    ret.emplace_back(make_mapping("Dpad Right",         Qt::Key::Key_6,         ControllerDeltas{.dpad_x = +1, .dpad_y =  0}));
    ret.emplace_back(make_mapping("Dpad Down+Right",    Qt::Key::Key_3,         ControllerDeltas{.dpad_x = +1, .dpad_y = +1}));
    ret.emplace_back(make_mapping("Dpad Down",          Qt::Key::Key_2,         ControllerDeltas{.dpad_x =  0, .dpad_y = +1}));
    ret.emplace_back(make_mapping("Dpad Down+Left",     Qt::Key::Key_1,         ControllerDeltas{.dpad_x = -1, .dpad_y = +1}));
    ret.emplace_back(make_mapping("Dpad Left",          Qt::Key::Key_4,         ControllerDeltas{.dpad_x = -1, .dpad_y =  0}));
    ret.emplace_back(make_mapping("Dpad Up+Left",       Qt::Key::Key_7,         ControllerDeltas{.dpad_x = -1, .dpad_y = -1}));

    ret.emplace_back(make_mapping("Left-Stick Up",      Qt::Key::Key_W,         ControllerDeltas{.left_x =  0, .left_y = -1}));
    ret.emplace_back(make_mapping("Left-Stick Right",   Qt::Key::Key_D,         ControllerDeltas{.left_x = +1, .left_y =  0}));
    ret.emplace_back(make_mapping("Left-Stick Down",    Qt::Key::Key_S,         ControllerDeltas{.left_x =  0, .left_y = +1}));
    ret.emplace_back(make_mapping("Left-Stick Left",    Qt::Key::Key_A,         ControllerDeltas{.left_x = -1, .left_y =  0}));
    ret.emplace_back(make_mapping("Right-Stick Up",     Qt::Key::Key_Up,        ControllerDeltas{.right_x =  0, .right_y = -1}));
    ret.emplace_back(make_mapping("Right-Stick Right",  Qt::Key::Key_Right,     ControllerDeltas{.right_x = +1, .right_y =  0}));
    ret.emplace_back(make_mapping("Right-Stick Down",   Qt::Key::Key_Down,      ControllerDeltas{.right_x =  0, .right_y = +1}));
    ret.emplace_back(make_mapping("Right-Stick Left",   Qt::Key::Key_Left,      ControllerDeltas{.right_x = -1, .right_y =  0}));

    ret.emplace_back(make_mapping("Y",              Qt::Key::Key_Slash,     ControllerDeltas{.buttons = BUTTON_Y}));
    ret.emplace_back(make_mapping("Y",              Qt::Key::Key_Question,  ControllerDeltas{.buttons = BUTTON_Y}));

    ret.emplace_back(make_mapping("B",              Qt::Key::Key_Shift,     ControllerDeltas{.buttons = BUTTON_B}));
    ret.emplace_back(make_mapping("B",              Qt::Key::Key_Control,   ControllerDeltas{.buttons = BUTTON_B}));

    ret.emplace_back(make_mapping("A",              Qt::Key::Key_Enter,     ControllerDeltas{.buttons = BUTTON_A}));
    ret.emplace_back(make_mapping("A",              Qt::Key::Key_Return,    ControllerDeltas{.buttons = BUTTON_A}));

    ret.emplace_back(make_mapping("X",              Qt::Key::Key_Apostrophe,ControllerDeltas{.buttons = BUTTON_X}));
    ret.emplace_back(make_mapping("X",              Qt::Key::Key_QuoteDbl,  ControllerDeltas{.buttons = BUTTON_X}));

    ret.emplace_back(make_mapping("L",              Qt::Key::Key_Q,         ControllerDeltas{.buttons = BUTTON_L}));
    ret.emplace_back(make_mapping("R",              Qt::Key::Key_E,         ControllerDeltas{.buttons = BUTTON_R}));
    ret.emplace_back(make_mapping("ZL",             Qt::Key::Key_R,         ControllerDeltas{.buttons = BUTTON_ZL}));
    ret.emplace_back(make_mapping("ZR",             Qt::Key::Key_Backslash, ControllerDeltas{.buttons = BUTTON_ZR}));
    ret.emplace_back(make_mapping("ZR",             Qt::Key::Key_Bar,       ControllerDeltas{.buttons = BUTTON_ZR}));

    ret.emplace_back(make_mapping("-",              Qt::Key::Key_Minus,     ControllerDeltas{.buttons = BUTTON_MINUS}));
    ret.emplace_back(make_mapping("-",              Qt::Key::Key_Underscore,ControllerDeltas{.buttons = BUTTON_MINUS}));
    ret.emplace_back(make_mapping("+",              Qt::Key::Key_Plus,      ControllerDeltas{.buttons = BUTTON_PLUS}));
    ret.emplace_back(make_mapping("+",              Qt::Key::Key_Equal,     ControllerDeltas{.buttons = BUTTON_PLUS}));

    ret.emplace_back(make_mapping("L-Click",        Qt::Key::Key_C,         ControllerDeltas{.buttons = BUTTON_LCLICK}));
    ret.emplace_back(make_mapping("R-Click",        Qt::Key::Key_0,         ControllerDeltas{.buttons = BUTTON_RCLICK}));

    ret.emplace_back(make_mapping("Home",           Qt::Key::Key_Home,      ControllerDeltas{.buttons = BUTTON_HOME}));
    ret.emplace_back(make_mapping("Home",           Qt::Key::Key_Escape,    ControllerDeltas{.buttons = BUTTON_HOME}));
    ret.emplace_back(make_mapping("Home",           Qt::Key::Key_H,         ControllerDeltas{.buttons = BUTTON_HOME}));

    ret.emplace_back(make_mapping("Capture",        Qt::Key::Key_Insert,    ControllerDeltas{.buttons = BUTTON_CAPTURE}));

    ret.emplace_back(make_mapping("A+R (for CFW)",  Qt::Key::Key_Y,         ControllerDeltas{.buttons = BUTTON_A | BUTTON_R}));

    return ret;
}




KeyboardMappingOption::~KeyboardMappingOption(){
    ADVANCED_MODE.remove_listener(*this);
}
KeyboardMappingOption::KeyboardMappingOption()
    : BatchOption(LockMode::UNLOCK_WHILE_RUNNING)
    , DESCRIPTION(
        "The following table is the mapping of keyboard keys to Switch controller presses. "
        "If you wish to remap a key, click on the cell in the \"Key\" column and press the desired key. "
        "You do not need to edit any of the other columns.<br><br>"
        "<font color=\"orange\">Note for keys that change behavior when combined with "
        "SHIFT or CTRL, you should include all of those combinations as well. "
        "For example, the default mapping for the Y button is both '/' and '?' "
        "because they are treated as different keys depending on whether SHIFT "
        "is held down. Letters are exempt from this as both lower and upper case "
        "letters are considered the same.</font>"
        "<br><br>"
        "Advanced users are free to edit the rest of the table. You can create "
        "new mappings or mappings that result in multiple buttons. "
        "For example, there is a special mapping for pressing A + R "
        "simultaneously that is useful for CFW users who are remotely "
        "controlling the program over Team Viewer."
    )
    , ADVANCED_MODE(
        "Unlock entire table (Advanced Mode):",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_STATIC(DESCRIPTION);
    PA_ADD_OPTION(ADVANCED_MODE);
    PA_ADD_OPTION(TABLE);
    ADVANCED_MODE.add_listener(*this);
}


void KeyboardMappingOption::load_json(const JsonValue& json){
    BatchOption::load_json(json);
    KeyboardMappingOption::value_changed(this);
}
void KeyboardMappingOption::value_changed(void* object){
    TABLE.set_advanced_mode(ADVANCED_MODE);
}












}
}
