/*  Controller Mapping: Keyboard -> Procon
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch/Controllers/NintendoSwitch_VirtualControllerState.h"
#include "NintendoSwitch_Joycon_from_Keyboard.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




std::vector<std::unique_ptr<EditableTableRow>> JoyconFromKeyboardTable::make_defaults(bool left){
    std::vector<std::unique_ptr<EditableTableRow>> ret;

    if (left){
        ret.emplace_back(make_mapping("Vertical: JS Up",    KeyboardKey::KEY_W,         JoyconDeltas{.joystick_x =  0, .joystick_y = -1}));
        ret.emplace_back(make_mapping("Vertical: JS Left",  KeyboardKey::KEY_A,         JoyconDeltas{.joystick_x = -1, .joystick_y =  0}));
        ret.emplace_back(make_mapping("Vertical: JS Down",  KeyboardKey::KEY_S,         JoyconDeltas{.joystick_x =  0, .joystick_y = +1}));
        ret.emplace_back(make_mapping("Vertical: JS Right", KeyboardKey::KEY_D,         JoyconDeltas{.joystick_x = +1, .joystick_y =  0}));

        ret.emplace_back(make_mapping("Vertical: Up",       KeyboardKey::KEY_UP,        JoyconDeltas{.buttons = BUTTON_UP}));
        ret.emplace_back(make_mapping("Vertical: Right",    KeyboardKey::KEY_RIGHT,     JoyconDeltas{.buttons = BUTTON_RIGHT}));
        ret.emplace_back(make_mapping("Vertical: Down",     KeyboardKey::KEY_DOWN,      JoyconDeltas{.buttons = BUTTON_DOWN}));
        ret.emplace_back(make_mapping("Vertical: Left",     KeyboardKey::KEY_LEFT,      JoyconDeltas{.buttons = BUTTON_LEFT}));

        ret.emplace_back(make_mapping("Vertical: L",        KeyboardKey::KEY_Q,         JoyconDeltas{.buttons = BUTTON_L}));
        ret.emplace_back(make_mapping("Vertical: ZL",       KeyboardKey::KEY_E,         JoyconDeltas{.buttons = BUTTON_ZL}));
        ret.emplace_back(make_mapping("Vertical: JS-Click", KeyboardKey::KEY_C,         JoyconDeltas{.buttons = BUTTON_LCLICK}));

        ret.emplace_back(make_mapping("Sideways: JS Up",    KeyboardKey::KEY_T,         JoyconDeltas{.joystick_x = +1, .joystick_y =  0}));
        ret.emplace_back(make_mapping("Sideways: JS Left",  KeyboardKey::KEY_F,         JoyconDeltas{.joystick_x =  0, .joystick_y = -1}));
        ret.emplace_back(make_mapping("Sideways: JS Down",  KeyboardKey::KEY_G,         JoyconDeltas{.joystick_x = -1, .joystick_y =  0}));
        ret.emplace_back(make_mapping("Sideways: JS Right", KeyboardKey::KEY_H,         JoyconDeltas{.joystick_x =  0, .joystick_y = +1}));

        ret.emplace_back(make_mapping("Sideways: Up",       KeyboardKey::KEY_APOSTROPHE,    JoyconDeltas{.buttons = BUTTON_RIGHT}));
        ret.emplace_back(make_mapping("Sideways: Right",    KeyboardKey::KEY_ENTER,         JoyconDeltas{.buttons = BUTTON_DOWN}));
        ret.emplace_back(make_mapping("Sideways: Down",     KeyboardKey::KEY_LEFT_SHIFT,    JoyconDeltas{.buttons = BUTTON_LEFT}));
        ret.emplace_back(make_mapping("Sideways: Down",     KeyboardKey::KEY_RIGHT_SHIFT,   JoyconDeltas{.buttons = BUTTON_LEFT}));
        ret.emplace_back(make_mapping("Sideways: Down",     KeyboardKey::KEY_LEFT_CTRL,     JoyconDeltas{.buttons = BUTTON_LEFT}));
        ret.emplace_back(make_mapping("Sideways: Down",     KeyboardKey::KEY_RIGHT_CTRL,    JoyconDeltas{.buttons = BUTTON_LEFT}));
        ret.emplace_back(make_mapping("Sideways: Left",     KeyboardKey::KEY_SLASH,         JoyconDeltas{.buttons = BUTTON_UP}));

        ret.emplace_back(make_mapping("Sideways: L",        KeyboardKey::KEY_R,         JoyconDeltas{.buttons = BUTTON_L}));
        ret.emplace_back(make_mapping("Sideways: ZL",       KeyboardKey::KEY_Y,         JoyconDeltas{.buttons = BUTTON_ZL}));
        ret.emplace_back(make_mapping("Sideways: JS-Click", KeyboardKey::KEY_N,         JoyconDeltas{.buttons = BUTTON_LCLICK}));

        //  Other
        ret.emplace_back(make_mapping("-",                  KeyboardKey::KEY_MINUS,     JoyconDeltas{.buttons = BUTTON_MINUS}));
        ret.emplace_back(make_mapping("Capture",            KeyboardKey::KEY_INSERT,    JoyconDeltas{.buttons = BUTTON_CAPTURE}));

        ret.emplace_back(make_mapping("SL",                 KeyboardKey::KEY_F1,        JoyconDeltas{.buttons = BUTTON_LEFT_SL}));
        ret.emplace_back(make_mapping("SR",                 KeyboardKey::KEY_F3,        JoyconDeltas{.buttons = BUTTON_LEFT_SR}));
    }else{
        ret.emplace_back(make_mapping("Vertical: JS Up",    KeyboardKey::KEY_W,         JoyconDeltas{.joystick_x =  0, .joystick_y = -1}));
        ret.emplace_back(make_mapping("Vertical: JS Left",  KeyboardKey::KEY_A,         JoyconDeltas{.joystick_x = -1, .joystick_y =  0}));
        ret.emplace_back(make_mapping("Vertical: JS Down",  KeyboardKey::KEY_S,         JoyconDeltas{.joystick_x =  0, .joystick_y = +1}));
        ret.emplace_back(make_mapping("Vertical: JS Right", KeyboardKey::KEY_D,         JoyconDeltas{.joystick_x = +1, .joystick_y =  0}));

        ret.emplace_back(make_mapping("Vertical: X",        KeyboardKey::KEY_APOSTROPHE,    JoyconDeltas{.buttons = BUTTON_X}));
        ret.emplace_back(make_mapping("Vertical: A",        KeyboardKey::KEY_ENTER,         JoyconDeltas{.buttons = BUTTON_A}));
        ret.emplace_back(make_mapping("Vertical: B",        KeyboardKey::KEY_LEFT_SHIFT,    JoyconDeltas{.buttons = BUTTON_B}));
        ret.emplace_back(make_mapping("Vertical: B",        KeyboardKey::KEY_RIGHT_SHIFT,   JoyconDeltas{.buttons = BUTTON_B}));
        ret.emplace_back(make_mapping("Vertical: B",        KeyboardKey::KEY_LEFT_CTRL,     JoyconDeltas{.buttons = BUTTON_B}));
        ret.emplace_back(make_mapping("Vertical: B",        KeyboardKey::KEY_RIGHT_CTRL,    JoyconDeltas{.buttons = BUTTON_B}));
        ret.emplace_back(make_mapping("Vertical: Y",        KeyboardKey::KEY_SLASH,         JoyconDeltas{.buttons = BUTTON_Y}));

        ret.emplace_back(make_mapping("Vertical: R",        KeyboardKey::KEY_Q,         JoyconDeltas{.buttons = BUTTON_R}));
        ret.emplace_back(make_mapping("Vertical: ZR",       KeyboardKey::KEY_E,         JoyconDeltas{.buttons = BUTTON_ZR}));
        ret.emplace_back(make_mapping("Vertical: JS-Click", KeyboardKey::KEY_C,         JoyconDeltas{.buttons = BUTTON_RCLICK}));

        ret.emplace_back(make_mapping("Sideways: JS Up",    KeyboardKey::KEY_T,         JoyconDeltas{.joystick_x = -1, .joystick_y =  0}));
        ret.emplace_back(make_mapping("Sideways: JS Left",  KeyboardKey::KEY_F,         JoyconDeltas{.joystick_x =  0, .joystick_y = +1}));
        ret.emplace_back(make_mapping("Sideways: JS Down",  KeyboardKey::KEY_G,         JoyconDeltas{.joystick_x = +1, .joystick_y =  0}));
        ret.emplace_back(make_mapping("Sideways: JS Right", KeyboardKey::KEY_H,         JoyconDeltas{.joystick_x =  0, .joystick_y = -1}));

        ret.emplace_back(make_mapping("Sideways: X",        KeyboardKey::KEY_UP,        JoyconDeltas{.buttons = BUTTON_Y}));
        ret.emplace_back(make_mapping("Sideways: A",        KeyboardKey::KEY_RIGHT,     JoyconDeltas{.buttons = BUTTON_X}));
        ret.emplace_back(make_mapping("Sideways: B",        KeyboardKey::KEY_DOWN,      JoyconDeltas{.buttons = BUTTON_A}));
        ret.emplace_back(make_mapping("Sideways: Y",        KeyboardKey::KEY_LEFT,      JoyconDeltas{.buttons = BUTTON_B}));

        ret.emplace_back(make_mapping("Sideways: R",        KeyboardKey::KEY_R,         JoyconDeltas{.buttons = BUTTON_R}));
        ret.emplace_back(make_mapping("Sideways: ZR",       KeyboardKey::KEY_Y,         JoyconDeltas{.buttons = BUTTON_ZR}));
        ret.emplace_back(make_mapping("Sideways: JS-Click", KeyboardKey::KEY_N,         JoyconDeltas{.buttons = BUTTON_RCLICK}));

        //  Other
        ret.emplace_back(make_mapping("+",                  KeyboardKey::KEY_EQUAL,     JoyconDeltas{.buttons = BUTTON_PLUS}));

        ret.emplace_back(make_mapping("Home",               KeyboardKey::KEY_HOME,      JoyconDeltas{.buttons = BUTTON_HOME}));
        ret.emplace_back(make_mapping("Home",               KeyboardKey::KEY_ESC,       JoyconDeltas{.buttons = BUTTON_HOME}));

        ret.emplace_back(make_mapping("SL",                 KeyboardKey::KEY_F1,        JoyconDeltas{.buttons = BUTTON_RIGHT_SL}));
        ret.emplace_back(make_mapping("SR",                 KeyboardKey::KEY_F3,        JoyconDeltas{.buttons = BUTTON_RIGHT_SR}));

        ret.emplace_back(make_mapping("C (Switch 2)",       KeyboardKey::KEY_B,         JoyconDeltas{.buttons = BUTTON_C}));
    }

    return ret;
};









JoyconFromKeyboardTableRow::JoyconFromKeyboardTableRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , label(false, LockMode::UNLOCK_WHILE_RUNNING, "", "")
    , key(LockMode::UNLOCK_WHILE_RUNNING)
    , buttons(LockMode::UNLOCK_WHILE_RUNNING, 0, 0, ((uint32_t)1 << TOTAL_BUTTONS) - 1)
    , joystick_x(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
    , joystick_y(LockMode::UNLOCK_WHILE_RUNNING, 0, -1, 1)
{
    add_option(label, "Description");
    add_option(key, "Key");
    add_option(buttons, "Button Bit-Field");
    add_option(joystick_x, "Joystick x");
    add_option(joystick_y, "Joystick y");
    set_advanced_mode(static_cast<JoyconFromKeyboardTable&>(parent_table).advanced_mode());
}
JoyconFromKeyboardTableRow::JoyconFromKeyboardTableRow(
    EditableTableOption& parent_table,
    bool advanced_mode,
    std::string description,
    KeyboardKey key,
    const JoyconDeltas& deltas
)
    : JoyconFromKeyboardTableRow(parent_table)
{
    label.set(std::move(description));
    this->key.set(key);
    buttons.set(deltas.buttons);
    joystick_x.set((int8_t)deltas.joystick_x);
    joystick_y.set((int8_t)deltas.joystick_y);
    set_advanced_mode(advanced_mode);
}
std::unique_ptr<EditableTableRow> JoyconFromKeyboardTableRow::clone() const{
    std::unique_ptr<JoyconFromKeyboardTableRow> ret(new JoyconFromKeyboardTableRow(parent()));
    ret->label.set(label);
    ret->key.set((KeyboardKey)key);
    ret->buttons.set(buttons);
    ret->joystick_x.set(joystick_x);
    ret->joystick_y.set(joystick_y);
    ret->set_advanced_mode(m_advanced_mode.load(std::memory_order_relaxed));
    return ret;
}
JoyconDeltas JoyconFromKeyboardTableRow::snapshot() const{
    return {
        .buttons = (Button)buttons.current_value(),
        .joystick_x = joystick_x,
        .joystick_y = joystick_y,
    };
}
void JoyconFromKeyboardTableRow::set_advanced_mode(bool enabled){
    m_advanced_mode.store(enabled, std::memory_order_relaxed);
    if (enabled){
        label.set_locked(false);
        buttons.set_visibility(ConfigOptionState::ENABLED);
        joystick_x.set_visibility(ConfigOptionState::ENABLED);
        joystick_y.set_visibility(ConfigOptionState::ENABLED);
    }else{
        label.set_locked(true);
        buttons.set_visibility(ConfigOptionState::DISABLED);
        joystick_x.set_visibility(ConfigOptionState::DISABLED);
        joystick_y.set_visibility(ConfigOptionState::DISABLED);
    }
}








JoyconFromKeyboardTable::JoyconFromKeyboardTable(bool left)
    : EditableTableOption_t<JoyconFromKeyboardTableRow>(
        left ? "<b>Left Joycon:</b>" : "<b>Right Joycon:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        true,
        make_defaults(left)
    )
    , m_advanced_mode(false)
{}
std::vector<std::string> JoyconFromKeyboardTable::make_header() const{
    return std::vector<std::string>{
        "Description",
        "Keyboard HID ID",
        "Button Bit-Field",
        "Joystick x",
        "Joystick y",
    };
}
void JoyconFromKeyboardTable::set_advanced_mode(bool enabled){
    m_advanced_mode.store(enabled, std::memory_order_relaxed);
    run_on_all_rows([enabled](JoyconFromKeyboardTableRow& row){
        row.set_advanced_mode(enabled);
        return false;
    });
}


std::unique_ptr<EditableTableRow> JoyconFromKeyboardTable::make_mapping(
    std::string description,
    KeyboardKey key,
    const JoyconDeltas& deltas
){
    return std::make_unique<JoyconFromKeyboardTableRow>(
        *this,
        m_advanced_mode.load(std::memory_order_relaxed),
        std::move(description),
        key,
        deltas
    );
}































}
}
