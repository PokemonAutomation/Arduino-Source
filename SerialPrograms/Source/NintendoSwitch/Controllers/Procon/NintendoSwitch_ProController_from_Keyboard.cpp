/*  Controller Mapping: Keyboard -> Procon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Controllers/NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch_ProController_from_Keyboard.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




std::vector<std::unique_ptr<EditableTableRow>> ProControllerFromKeyboardTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;

    ret.emplace_back(make_mapping("Dpad Up",            KeyboardKey::KEY_KP_8,      ProControllerDeltas{.dpad_x =  0, .dpad_y = -1}));
    ret.emplace_back(make_mapping("Dpad Up+Right",      KeyboardKey::KEY_KP_9,      ProControllerDeltas{.dpad_x = +1, .dpad_y = -1}));
    ret.emplace_back(make_mapping("Dpad Right",         KeyboardKey::KEY_KP_6,      ProControllerDeltas{.dpad_x = +1, .dpad_y =  0}));
    ret.emplace_back(make_mapping("Dpad Down+Right",    KeyboardKey::KEY_KP_3,      ProControllerDeltas{.dpad_x = +1, .dpad_y = +1}));
    ret.emplace_back(make_mapping("Dpad Down",          KeyboardKey::KEY_KP_2,      ProControllerDeltas{.dpad_x =  0, .dpad_y = +1}));
    ret.emplace_back(make_mapping("Dpad Down+Left",     KeyboardKey::KEY_KP_1,      ProControllerDeltas{.dpad_x = -1, .dpad_y = +1}));
    ret.emplace_back(make_mapping("Dpad Left",          KeyboardKey::KEY_KP_4,      ProControllerDeltas{.dpad_x = -1, .dpad_y =  0}));
    ret.emplace_back(make_mapping("Dpad Up+Left",       KeyboardKey::KEY_KP_7,      ProControllerDeltas{.dpad_x = -1, .dpad_y = -1}));

    ret.emplace_back(make_mapping("Left-Stick Up",      KeyboardKey::KEY_W,         ProControllerDeltas{.left_x =  0, .left_y = -1}));
    ret.emplace_back(make_mapping("Left-Stick Left",    KeyboardKey::KEY_A,         ProControllerDeltas{.left_x = -1, .left_y =  0}));
    ret.emplace_back(make_mapping("Left-Stick Down",    KeyboardKey::KEY_S,         ProControllerDeltas{.left_x =  0, .left_y = +1}));
    ret.emplace_back(make_mapping("Left-Stick Right",   KeyboardKey::KEY_D,         ProControllerDeltas{.left_x = +1, .left_y =  0}));
    ret.emplace_back(make_mapping("Right-Stick Up",     KeyboardKey::KEY_UP,        ProControllerDeltas{.right_x =  0, .right_y = -1}));
    ret.emplace_back(make_mapping("Right-Stick Right",  KeyboardKey::KEY_RIGHT,     ProControllerDeltas{.right_x = +1, .right_y =  0}));
    ret.emplace_back(make_mapping("Right-Stick Down",   KeyboardKey::KEY_DOWN,      ProControllerDeltas{.right_x =  0, .right_y = +1}));
    ret.emplace_back(make_mapping("Right-Stick Left",   KeyboardKey::KEY_LEFT,      ProControllerDeltas{.right_x = -1, .right_y =  0}));

    ret.emplace_back(make_mapping("Y",              KeyboardKey::KEY_SLASH,         ProControllerDeltas{.buttons = BUTTON_Y}));

    ret.emplace_back(make_mapping("B",              KeyboardKey::KEY_LEFT_SHIFT,    ProControllerDeltas{.buttons = BUTTON_B}));
    ret.emplace_back(make_mapping("B",              KeyboardKey::KEY_RIGHT_SHIFT,   ProControllerDeltas{.buttons = BUTTON_B}));
    ret.emplace_back(make_mapping("B",              KeyboardKey::KEY_LEFT_CTRL,     ProControllerDeltas{.buttons = BUTTON_B}));
    ret.emplace_back(make_mapping("B",              KeyboardKey::KEY_RIGHT_CTRL,    ProControllerDeltas{.buttons = BUTTON_B}));

    ret.emplace_back(make_mapping("A",              KeyboardKey::KEY_ENTER,         ProControllerDeltas{.buttons = BUTTON_A}));
    ret.emplace_back(make_mapping("A",              KeyboardKey::KEY_KP_ENTER,      ProControllerDeltas{.buttons = BUTTON_A}));

    ret.emplace_back(make_mapping("X",              KeyboardKey::KEY_APOSTROPHE,    ProControllerDeltas{.buttons = BUTTON_X}));

    ret.emplace_back(make_mapping("L",              KeyboardKey::KEY_Q,             ProControllerDeltas{.buttons = BUTTON_L}));
    ret.emplace_back(make_mapping("R",              KeyboardKey::KEY_E,             ProControllerDeltas{.buttons = BUTTON_R}));
    ret.emplace_back(make_mapping("ZL",             KeyboardKey::KEY_R,             ProControllerDeltas{.buttons = BUTTON_ZL}));
    ret.emplace_back(make_mapping("R",              KeyboardKey::KEY_RIGHT_BRACE,   ProControllerDeltas{.buttons = BUTTON_R}));
    ret.emplace_back(make_mapping("ZR",             KeyboardKey::KEY_BACKSLASH,     ProControllerDeltas{.buttons = BUTTON_ZR}));

    ret.emplace_back(make_mapping("-",              KeyboardKey::KEY_MINUS,         ProControllerDeltas{.buttons = BUTTON_MINUS}));
    ret.emplace_back(make_mapping("-",              KeyboardKey::KEY_KP_MINUS,      ProControllerDeltas{.buttons = BUTTON_MINUS}));
    ret.emplace_back(make_mapping("+",              KeyboardKey::KEY_EQUAL,         ProControllerDeltas{.buttons = BUTTON_PLUS}));
    ret.emplace_back(make_mapping("+",              KeyboardKey::KEY_KP_PLUS,       ProControllerDeltas{.buttons = BUTTON_PLUS}));

    ret.emplace_back(make_mapping("L-Click",        KeyboardKey::KEY_C,             ProControllerDeltas{.buttons = BUTTON_LCLICK}));
    ret.emplace_back(make_mapping("R-Click",        KeyboardKey::KEY_KP_0,          ProControllerDeltas{.buttons = BUTTON_RCLICK}));

    ret.emplace_back(make_mapping("Home",           KeyboardKey::KEY_HOME,          ProControllerDeltas{.buttons = BUTTON_HOME}));
    ret.emplace_back(make_mapping("Home",           KeyboardKey::KEY_ESC,           ProControllerDeltas{.buttons = BUTTON_HOME}));
    ret.emplace_back(make_mapping("Home",           KeyboardKey::KEY_H,             ProControllerDeltas{.buttons = BUTTON_HOME}));

    ret.emplace_back(make_mapping("Capture",        KeyboardKey::KEY_INSERT,        ProControllerDeltas{.buttons = BUTTON_CAPTURE}));

    ret.emplace_back(make_mapping("C (Switch 2)",   KeyboardKey::KEY_B,             ProControllerDeltas{.buttons = BUTTON_C}));
    ret.emplace_back(make_mapping("GL (Switch 2)",  KeyboardKey::KEY_V,             ProControllerDeltas{.buttons = BUTTON_GL}));
    ret.emplace_back(make_mapping("GR (Switch 2)",  KeyboardKey::KEY_DOT,           ProControllerDeltas{.buttons = BUTTON_GR}));

    ret.emplace_back(make_mapping("A+R (for CFW)",  KeyboardKey::KEY_Y,             ProControllerDeltas{.buttons = BUTTON_A | BUTTON_R}));

    return ret;
}









ProControllerFromKeyboardTableRow::ProControllerFromKeyboardTableRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , label(false, LockMode::UNLOCK_WHILE_RUNNING, "", "")
    , key(LockMode::UNLOCK_WHILE_RUNNING)
    , buttons(LockMode::UNLOCK_WHILE_RUNNING, 0, 0, ((uint32_t)1 << TOTAL_BUTTONS) - 1)
    , dpad_x(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
    , dpad_y(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
    , left_stick_x(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
    , left_stick_y(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
    , right_stick_x(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
    , right_stick_y(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
{
    add_option(label, "Description");
    add_option(key, "HID ID");
    add_option(buttons, "Button Bit-Field");
    add_option(dpad_x, "Dpad x");
    add_option(dpad_y, "Dpad y");
    add_option(left_stick_x, "Left-Stick x");
    add_option(left_stick_y, "Left-Stick y");
    add_option(right_stick_x, "Right-Stick x");
    add_option(right_stick_y, "Right-Stick y");
    set_advanced_mode(static_cast<ProControllerFromKeyboardTable&>(parent_table).advanced_mode());
}
ProControllerFromKeyboardTableRow::ProControllerFromKeyboardTableRow(
    EditableTableOption& parent_table,
    bool advanced_mode,
    std::string description,
    KeyboardKey key,
    const ProControllerDeltas& deltas
)
    : ProControllerFromKeyboardTableRow(parent_table)
{
    label.set(std::move(description));
    this->key.set(key);
    buttons.set(deltas.buttons);
    dpad_x.set((int8_t)deltas.dpad_x);
    dpad_y.set((int8_t)deltas.dpad_y);
    left_stick_x.set((int8_t)deltas.left_x);
    left_stick_y.set((int8_t)deltas.left_y);
    right_stick_x.set((int8_t)deltas.right_x);
    right_stick_y.set((int8_t)deltas.right_y);
    set_advanced_mode(advanced_mode);
}
std::unique_ptr<EditableTableRow> ProControllerFromKeyboardTableRow::clone() const{
    std::unique_ptr<ProControllerFromKeyboardTableRow> ret(new ProControllerFromKeyboardTableRow(parent()));
    ret->label.set(label);
    ret->key.set((KeyboardKey)key);
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
ProControllerDeltas ProControllerFromKeyboardTableRow::snapshot() const{
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
void ProControllerFromKeyboardTableRow::set_advanced_mode(bool enabled){
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








ProControllerFromKeyboardTable::ProControllerFromKeyboardTable()
    : EditableTableOption_t<ProControllerFromKeyboardTableRow>(
        "<b>Pro Controller:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        true,
        make_defaults()
    )
    , m_advanced_mode(false)
{}
std::vector<std::string> ProControllerFromKeyboardTable::make_header() const{
    return std::vector<std::string>{
        "Description",
        "Keyboard HID ID",
        "Button Bit-Field",
        "Dpad x",
        "Dpad y",
        "Left-Stick x",
        "Left-Stick y",
        "Right-Stick x",
        "Right-Stick y",
    };
}
void ProControllerFromKeyboardTable::set_advanced_mode(bool enabled){
    m_advanced_mode.store(enabled, std::memory_order_relaxed);
    run_on_all_rows([enabled](ProControllerFromKeyboardTableRow& row){
        row.set_advanced_mode(enabled);
        return false;
    });
}


std::unique_ptr<EditableTableRow> ProControllerFromKeyboardTable::make_mapping(
    std::string description,
    KeyboardKey key,
    const ProControllerDeltas& deltas
){
    return std::make_unique<ProControllerFromKeyboardTableRow>(
        *this,
        m_advanced_mode.load(std::memory_order_relaxed),
        std::move(description),
        key,
        deltas
    );
}































}
}
