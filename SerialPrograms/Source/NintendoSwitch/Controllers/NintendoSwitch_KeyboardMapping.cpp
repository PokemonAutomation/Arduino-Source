/*  Nintendo Keyboard Mapping
 *
 *  From: https://github.com/PokemonAutomation/
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


ProControllerKeyMapTableRow::ProControllerKeyMapTableRow(EditableTableOption& parent_table)
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
    add_option(key, "Key");
    add_option(buttons, "Button Bit-Field");
    add_option(dpad_x, "Dpad x");
    add_option(dpad_y, "Dpad y");
    add_option(left_stick_x, "Left-Stick x");
    add_option(left_stick_y, "Left-Stick y");
    add_option(right_stick_x, "Right-Stick x");
    add_option(right_stick_y, "Right-Stick y");
    set_advanced_mode(static_cast<ProControllerKeyboardMappingTable&>(parent_table).advanced_mode());
}
ProControllerKeyMapTableRow::ProControllerKeyMapTableRow(
    EditableTableOption& parent_table,
    bool advanced_mode,
    std::string description,
    Qt::Key key,
    const ProControllerDeltas& deltas
)
    : ProControllerKeyMapTableRow(parent_table)
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
std::unique_ptr<EditableTableRow> ProControllerKeyMapTableRow::clone() const{
    std::unique_ptr<ProControllerKeyMapTableRow> ret(new ProControllerKeyMapTableRow(parent()));
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
ProControllerDeltas ProControllerKeyMapTableRow::snapshot() const{
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
void ProControllerKeyMapTableRow::set_advanced_mode(bool enabled){
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





ProControllerKeyboardMappingTable::ProControllerKeyboardMappingTable()
    : EditableTableOption_t<ProControllerKeyMapTableRow>(
        "<b>Pro Controller:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        true,
        make_defaults()
    )
    , m_advanced_mode(false)
{}
std::vector<std::string> ProControllerKeyboardMappingTable::make_header() const{
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
void ProControllerKeyboardMappingTable::set_advanced_mode(bool enabled){
    m_advanced_mode.store(enabled, std::memory_order_relaxed);
    run_on_all_rows([enabled](ProControllerKeyMapTableRow& row){
        row.set_advanced_mode(enabled);
        return false;
    });
}


std::unique_ptr<EditableTableRow> ProControllerKeyboardMappingTable::make_mapping(
    std::string description,
    Qt::Key key,
    const ProControllerDeltas& deltas
){
    return std::make_unique<ProControllerKeyMapTableRow>(
        *this,
        m_advanced_mode.load(std::memory_order_relaxed),
        std::move(description),
        key,
        deltas
    );
}
std::vector<std::unique_ptr<EditableTableRow>> ProControllerKeyboardMappingTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;

    ret.emplace_back(make_mapping("Dpad Up",            Qt::Key::Key_8,         ProControllerDeltas{.dpad_x =  0, .dpad_y = -1}));
    ret.emplace_back(make_mapping("Dpad Up+Right",      Qt::Key::Key_9,         ProControllerDeltas{.dpad_x = +1, .dpad_y = -1}));
    ret.emplace_back(make_mapping("Dpad Right",         Qt::Key::Key_6,         ProControllerDeltas{.dpad_x = +1, .dpad_y =  0}));
    ret.emplace_back(make_mapping("Dpad Down+Right",    Qt::Key::Key_3,         ProControllerDeltas{.dpad_x = +1, .dpad_y = +1}));
    ret.emplace_back(make_mapping("Dpad Down",          Qt::Key::Key_2,         ProControllerDeltas{.dpad_x =  0, .dpad_y = +1}));
    ret.emplace_back(make_mapping("Dpad Down+Left",     Qt::Key::Key_1,         ProControllerDeltas{.dpad_x = -1, .dpad_y = +1}));
    ret.emplace_back(make_mapping("Dpad Left",          Qt::Key::Key_4,         ProControllerDeltas{.dpad_x = -1, .dpad_y =  0}));
    ret.emplace_back(make_mapping("Dpad Up+Left",       Qt::Key::Key_7,         ProControllerDeltas{.dpad_x = -1, .dpad_y = -1}));

    ret.emplace_back(make_mapping("Left-Stick Up",      Qt::Key::Key_W,         ProControllerDeltas{.left_x =  0, .left_y = -1}));
    ret.emplace_back(make_mapping("Left-Stick Left",    Qt::Key::Key_A,         ProControllerDeltas{.left_x = -1, .left_y =  0}));
    ret.emplace_back(make_mapping("Left-Stick Down",    Qt::Key::Key_S,         ProControllerDeltas{.left_x =  0, .left_y = +1}));
    ret.emplace_back(make_mapping("Left-Stick Right",   Qt::Key::Key_D,         ProControllerDeltas{.left_x = +1, .left_y =  0}));
    ret.emplace_back(make_mapping("Right-Stick Up",     Qt::Key::Key_Up,        ProControllerDeltas{.right_x =  0, .right_y = -1}));
    ret.emplace_back(make_mapping("Right-Stick Right",  Qt::Key::Key_Right,     ProControllerDeltas{.right_x = +1, .right_y =  0}));
    ret.emplace_back(make_mapping("Right-Stick Down",   Qt::Key::Key_Down,      ProControllerDeltas{.right_x =  0, .right_y = +1}));
    ret.emplace_back(make_mapping("Right-Stick Left",   Qt::Key::Key_Left,      ProControllerDeltas{.right_x = -1, .right_y =  0}));

    ret.emplace_back(make_mapping("Y",              Qt::Key::Key_Slash,     ProControllerDeltas{.buttons = BUTTON_Y}));
    ret.emplace_back(make_mapping("Y",              Qt::Key::Key_Question,  ProControllerDeltas{.buttons = BUTTON_Y}));

    ret.emplace_back(make_mapping("B",              Qt::Key::Key_Shift,     ProControllerDeltas{.buttons = BUTTON_B}));
    ret.emplace_back(make_mapping("B",              Qt::Key::Key_Control,   ProControllerDeltas{.buttons = BUTTON_B}));

    ret.emplace_back(make_mapping("A",              Qt::Key::Key_Enter,     ProControllerDeltas{.buttons = BUTTON_A}));
    ret.emplace_back(make_mapping("A",              Qt::Key::Key_Return,    ProControllerDeltas{.buttons = BUTTON_A}));

    ret.emplace_back(make_mapping("X",              Qt::Key::Key_Apostrophe,ProControllerDeltas{.buttons = BUTTON_X}));
    ret.emplace_back(make_mapping("X",              Qt::Key::Key_QuoteDbl,  ProControllerDeltas{.buttons = BUTTON_X}));

    ret.emplace_back(make_mapping("L",              Qt::Key::Key_Q,             ProControllerDeltas{.buttons = BUTTON_L}));
    ret.emplace_back(make_mapping("R",              Qt::Key::Key_E,             ProControllerDeltas{.buttons = BUTTON_R}));
    ret.emplace_back(make_mapping("ZL",             Qt::Key::Key_R,             ProControllerDeltas{.buttons = BUTTON_ZL}));
    ret.emplace_back(make_mapping("R",              Qt::Key::Key_BraceRight,    ProControllerDeltas{.buttons = BUTTON_R}));
    ret.emplace_back(make_mapping("R",              Qt::Key::Key_BracketRight,  ProControllerDeltas{.buttons = BUTTON_R}));
    ret.emplace_back(make_mapping("ZR",             Qt::Key::Key_Backslash,     ProControllerDeltas{.buttons = BUTTON_ZR}));
    ret.emplace_back(make_mapping("ZR",             Qt::Key::Key_Bar,           ProControllerDeltas{.buttons = BUTTON_ZR}));

    ret.emplace_back(make_mapping("-",              Qt::Key::Key_Minus,         ProControllerDeltas{.buttons = BUTTON_MINUS}));
    ret.emplace_back(make_mapping("-",              Qt::Key::Key_Underscore,    ProControllerDeltas{.buttons = BUTTON_MINUS}));
    ret.emplace_back(make_mapping("+",              Qt::Key::Key_Plus,          ProControllerDeltas{.buttons = BUTTON_PLUS}));
    ret.emplace_back(make_mapping("+",              Qt::Key::Key_Equal,         ProControllerDeltas{.buttons = BUTTON_PLUS}));

    ret.emplace_back(make_mapping("L-Click",        Qt::Key::Key_C,             ProControllerDeltas{.buttons = BUTTON_LCLICK}));
    ret.emplace_back(make_mapping("R-Click",        Qt::Key::Key_0,             ProControllerDeltas{.buttons = BUTTON_RCLICK}));

    ret.emplace_back(make_mapping("Home",           Qt::Key::Key_Home,          ProControllerDeltas{.buttons = BUTTON_HOME}));
    ret.emplace_back(make_mapping("Home",           Qt::Key::Key_Escape,        ProControllerDeltas{.buttons = BUTTON_HOME}));
    ret.emplace_back(make_mapping("Home",           Qt::Key::Key_H,             ProControllerDeltas{.buttons = BUTTON_HOME}));

    ret.emplace_back(make_mapping("Capture",        Qt::Key::Key_Insert,        ProControllerDeltas{.buttons = BUTTON_CAPTURE}));

    ret.emplace_back(make_mapping("C (Switch 2)",   Qt::Key::Key_B,             ProControllerDeltas{.buttons = BUTTON_C}));
    ret.emplace_back(make_mapping("GL (Switch 2)",  Qt::Key::Key_V,             ProControllerDeltas{.buttons = BUTTON_GL}));
    ret.emplace_back(make_mapping("GR (Switch 2)",  Qt::Key::Key_Period,        ProControllerDeltas{.buttons = BUTTON_GR}));
    ret.emplace_back(make_mapping("GR (Switch 2)",  Qt::Key::Key_Greater,       ProControllerDeltas{.buttons = BUTTON_GR}));

    ret.emplace_back(make_mapping("A+R (for CFW)",  Qt::Key::Key_Y,             ProControllerDeltas{.buttons = BUTTON_A | BUTTON_R}));

    return ret;
}




JoyconKeyMapTableRow::JoyconKeyMapTableRow(EditableTableOption& parent_table)
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
    set_advanced_mode(static_cast<ProControllerKeyboardMappingTable&>(parent_table).advanced_mode());
}
JoyconKeyMapTableRow::JoyconKeyMapTableRow(
    EditableTableOption& parent_table,
    bool advanced_mode,
    std::string description,
    Qt::Key key,
    const JoyconDeltas& deltas
)
    : JoyconKeyMapTableRow(parent_table)
{
    label.set(std::move(description));
    this->key.set(key);
    buttons.set(deltas.buttons);
    joystick_x.set(deltas.joystick_x);
    joystick_y.set(deltas.joystick_y);
    set_advanced_mode(advanced_mode);
}
std::unique_ptr<EditableTableRow> JoyconKeyMapTableRow::clone() const{
    std::unique_ptr<JoyconKeyMapTableRow> ret(new JoyconKeyMapTableRow(parent()));
    ret->label.set(label);
    ret->key.set((uint32_t)key);
    ret->buttons.set(buttons);
    ret->joystick_x.set(joystick_x);
    ret->joystick_y.set(joystick_y);
    ret->set_advanced_mode(m_advanced_mode.load(std::memory_order_relaxed));
    return ret;
}
JoyconDeltas JoyconKeyMapTableRow::snapshot() const{
    return {
        .buttons = (Button)buttons.current_value(),
        .joystick_x = joystick_x,
        .joystick_y = joystick_y,
    };
}
void JoyconKeyMapTableRow::set_advanced_mode(bool enabled){
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



JoyconKeyboardMappingTable::JoyconKeyboardMappingTable(bool left)
    : EditableTableOption_t<JoyconKeyMapTableRow>(
        left ? "<b>Left Joycon:</b>" : "<b>Right Joycon:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        true,
        make_defaults(left)
    )
    , m_advanced_mode(false)
{}
std::vector<std::string> JoyconKeyboardMappingTable::make_header() const{
    return std::vector<std::string>{
        "Description",
        "Key",
        "Button Bit-Field",
        "Joystick x",
        "Joystick y",
    };
}
void JoyconKeyboardMappingTable::set_advanced_mode(bool enabled){
    m_advanced_mode.store(enabled, std::memory_order_relaxed);
    run_on_all_rows([enabled](JoyconKeyMapTableRow& row){
        row.set_advanced_mode(enabled);
        return false;
    });
}


std::unique_ptr<EditableTableRow> JoyconKeyboardMappingTable::make_mapping(
    std::string description,
    Qt::Key key,
    const JoyconDeltas& deltas
){
    return std::make_unique<JoyconKeyMapTableRow>(
        *this,
        m_advanced_mode.load(std::memory_order_relaxed),
        std::move(description),
        key,
        deltas
    );
}
std::vector<std::unique_ptr<EditableTableRow>> JoyconKeyboardMappingTable::make_defaults(bool left){
    std::vector<std::unique_ptr<EditableTableRow>> ret;

    if (left){
        ret.emplace_back(make_mapping("Vertical: JS Up",    Qt::Key::Key_W,         JoyconDeltas{.joystick_x =  0, .joystick_y = -1}));
        ret.emplace_back(make_mapping("Vertical: JS Left",  Qt::Key::Key_A,         JoyconDeltas{.joystick_x = -1, .joystick_y =  0}));
        ret.emplace_back(make_mapping("Vertical: JS Down",  Qt::Key::Key_S,         JoyconDeltas{.joystick_x =  0, .joystick_y = +1}));
        ret.emplace_back(make_mapping("Vertical: JS Right", Qt::Key::Key_D,         JoyconDeltas{.joystick_x = +1, .joystick_y =  0}));

        ret.emplace_back(make_mapping("Vertical: Up",       Qt::Key::Key_Up,        JoyconDeltas{.buttons = BUTTON_UP}));
        ret.emplace_back(make_mapping("Vertical: Right",    Qt::Key::Key_Right,     JoyconDeltas{.buttons = BUTTON_RIGHT}));
        ret.emplace_back(make_mapping("Vertical: Down",     Qt::Key::Key_Down,      JoyconDeltas{.buttons = BUTTON_DOWN}));
        ret.emplace_back(make_mapping("Vertical: Left",     Qt::Key::Key_Left,      JoyconDeltas{.buttons = BUTTON_LEFT}));

        ret.emplace_back(make_mapping("Vertical: L",        Qt::Key::Key_Q,         JoyconDeltas{.buttons = BUTTON_L}));
        ret.emplace_back(make_mapping("Vertical: ZL",       Qt::Key::Key_E,         JoyconDeltas{.buttons = BUTTON_ZL}));
        ret.emplace_back(make_mapping("Vertical: JS-Click", Qt::Key::Key_C,         JoyconDeltas{.buttons = BUTTON_LCLICK}));

        ret.emplace_back(make_mapping("Sideways: JS Up",    Qt::Key::Key_T,         JoyconDeltas{.joystick_x = +1, .joystick_y =  0}));
        ret.emplace_back(make_mapping("Sideways: JS Left",  Qt::Key::Key_F,         JoyconDeltas{.joystick_x =  0, .joystick_y = -1}));
        ret.emplace_back(make_mapping("Sideways: JS Down",  Qt::Key::Key_G,         JoyconDeltas{.joystick_x = -1, .joystick_y =  0}));
        ret.emplace_back(make_mapping("Sideways: JS Right", Qt::Key::Key_H,         JoyconDeltas{.joystick_x =  0, .joystick_y = +1}));

        ret.emplace_back(make_mapping("Sideways: Up",       Qt::Key::Key_Apostrophe,JoyconDeltas{.buttons = BUTTON_RIGHT}));
        ret.emplace_back(make_mapping("Sideways: Up",       Qt::Key::Key_QuoteDbl,  JoyconDeltas{.buttons = BUTTON_RIGHT}));
        ret.emplace_back(make_mapping("Sideways: Right",    Qt::Key::Key_Enter,     JoyconDeltas{.buttons = BUTTON_DOWN}));
        ret.emplace_back(make_mapping("Sideways: Right",    Qt::Key::Key_Return,    JoyconDeltas{.buttons = BUTTON_DOWN}));
        ret.emplace_back(make_mapping("Sideways: Down",     Qt::Key::Key_Shift,     JoyconDeltas{.buttons = BUTTON_LEFT}));
        ret.emplace_back(make_mapping("Sideways: Down",     Qt::Key::Key_Control,   JoyconDeltas{.buttons = BUTTON_LEFT}));
        ret.emplace_back(make_mapping("Sideways: Left",     Qt::Key::Key_Slash,     JoyconDeltas{.buttons = BUTTON_UP}));
        ret.emplace_back(make_mapping("Sideways: Left",     Qt::Key::Key_Question,  JoyconDeltas{.buttons = BUTTON_UP}));

        ret.emplace_back(make_mapping("Sideways: L",        Qt::Key::Key_R,         JoyconDeltas{.buttons = BUTTON_L}));
        ret.emplace_back(make_mapping("Sideways: ZL",       Qt::Key::Key_Y,         JoyconDeltas{.buttons = BUTTON_ZL}));
        ret.emplace_back(make_mapping("Sideways: JS-Click", Qt::Key::Key_N,         JoyconDeltas{.buttons = BUTTON_LCLICK}));

        //  Other
        ret.emplace_back(make_mapping("-",              Qt::Key::Key_Minus,     JoyconDeltas{.buttons = BUTTON_MINUS}));
        ret.emplace_back(make_mapping("-",              Qt::Key::Key_Underscore,JoyconDeltas{.buttons = BUTTON_MINUS}));

        ret.emplace_back(make_mapping("Capture",        Qt::Key::Key_Insert,    JoyconDeltas{.buttons = BUTTON_CAPTURE}));

        ret.emplace_back(make_mapping("SL",             Qt::Key::Key_F1,        JoyconDeltas{.buttons = BUTTON_LEFT_SL}));
        ret.emplace_back(make_mapping("SR",             Qt::Key::Key_F3,        JoyconDeltas{.buttons = BUTTON_LEFT_SR}));
    }else{
        ret.emplace_back(make_mapping("Vertical: JS Up",    Qt::Key::Key_W,         JoyconDeltas{.joystick_x =  0, .joystick_y = -1}));
        ret.emplace_back(make_mapping("Vertical: JS Left",  Qt::Key::Key_A,         JoyconDeltas{.joystick_x = -1, .joystick_y =  0}));
        ret.emplace_back(make_mapping("Vertical: JS Down",  Qt::Key::Key_S,         JoyconDeltas{.joystick_x =  0, .joystick_y = +1}));
        ret.emplace_back(make_mapping("Vertical: JS Right", Qt::Key::Key_D,         JoyconDeltas{.joystick_x = +1, .joystick_y =  0}));

        ret.emplace_back(make_mapping("Vertical: X",        Qt::Key::Key_Apostrophe,JoyconDeltas{.buttons = BUTTON_X}));
        ret.emplace_back(make_mapping("Vertical: X",        Qt::Key::Key_QuoteDbl,  JoyconDeltas{.buttons = BUTTON_X}));

        ret.emplace_back(make_mapping("Vertical: A",        Qt::Key::Key_Enter,     JoyconDeltas{.buttons = BUTTON_A}));
        ret.emplace_back(make_mapping("Vertical: A",        Qt::Key::Key_Return,    JoyconDeltas{.buttons = BUTTON_A}));

        ret.emplace_back(make_mapping("Vertical: B",        Qt::Key::Key_Shift,     JoyconDeltas{.buttons = BUTTON_B}));
        ret.emplace_back(make_mapping("Vertical: B",        Qt::Key::Key_Control,   JoyconDeltas{.buttons = BUTTON_B}));

        ret.emplace_back(make_mapping("Vertical: Y",        Qt::Key::Key_Slash,     JoyconDeltas{.buttons = BUTTON_Y}));
        ret.emplace_back(make_mapping("Vertical: Y",        Qt::Key::Key_Question,  JoyconDeltas{.buttons = BUTTON_Y}));

        ret.emplace_back(make_mapping("Vertical: R",        Qt::Key::Key_Q,         JoyconDeltas{.buttons = BUTTON_R}));
        ret.emplace_back(make_mapping("Vertical: ZR",       Qt::Key::Key_E,         JoyconDeltas{.buttons = BUTTON_ZR}));
        ret.emplace_back(make_mapping("Vertical: JS-Click", Qt::Key::Key_C,         JoyconDeltas{.buttons = BUTTON_RCLICK}));

        ret.emplace_back(make_mapping("Sideways: JS Up",    Qt::Key::Key_T,         JoyconDeltas{.joystick_x = -1, .joystick_y =  0}));
        ret.emplace_back(make_mapping("Sideways: JS Left",  Qt::Key::Key_F,         JoyconDeltas{.joystick_x =  0, .joystick_y = +1}));
        ret.emplace_back(make_mapping("Sideways: JS Down",  Qt::Key::Key_G,         JoyconDeltas{.joystick_x = +1, .joystick_y =  0}));
        ret.emplace_back(make_mapping("Sideways: JS Right", Qt::Key::Key_H,         JoyconDeltas{.joystick_x =  0, .joystick_y = -1}));

        ret.emplace_back(make_mapping("Sideways: X",        Qt::Key::Key_Up,        JoyconDeltas{.buttons = BUTTON_Y}));
        ret.emplace_back(make_mapping("Sideways: A",        Qt::Key::Key_Right,     JoyconDeltas{.buttons = BUTTON_X}));
        ret.emplace_back(make_mapping("Sideways: B",        Qt::Key::Key_Down,      JoyconDeltas{.buttons = BUTTON_A}));
        ret.emplace_back(make_mapping("Sideways: Y",        Qt::Key::Key_Left,      JoyconDeltas{.buttons = BUTTON_B}));

        ret.emplace_back(make_mapping("Sideways: R",        Qt::Key::Key_R,         JoyconDeltas{.buttons = BUTTON_R}));
        ret.emplace_back(make_mapping("Sideways: ZR",       Qt::Key::Key_Y,         JoyconDeltas{.buttons = BUTTON_ZR}));
        ret.emplace_back(make_mapping("Sideways: JS-Click", Qt::Key::Key_N,         JoyconDeltas{.buttons = BUTTON_RCLICK}));

        //  Other
        ret.emplace_back(make_mapping("+",              Qt::Key::Key_Plus,      JoyconDeltas{.buttons = BUTTON_PLUS}));
        ret.emplace_back(make_mapping("+",              Qt::Key::Key_Equal,     JoyconDeltas{.buttons = BUTTON_PLUS}));

        ret.emplace_back(make_mapping("Home",           Qt::Key::Key_Home,      JoyconDeltas{.buttons = BUTTON_HOME}));
        ret.emplace_back(make_mapping("Home",           Qt::Key::Key_Escape,    JoyconDeltas{.buttons = BUTTON_HOME}));

        ret.emplace_back(make_mapping("SL",             Qt::Key::Key_F1,        JoyconDeltas{.buttons = BUTTON_RIGHT_SL}));
        ret.emplace_back(make_mapping("SR",             Qt::Key::Key_F3,        JoyconDeltas{.buttons = BUTTON_RIGHT_SR}));

        ret.emplace_back(make_mapping("C (Switch 2)",   Qt::Key::Key_B,         JoyconDeltas{.buttons = BUTTON_C}));
    }

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
    , LEFT_JOYCON0(true)
    , RIGHT_JOYCON0(false)
{
    PA_ADD_STATIC(DESCRIPTION);
    PA_ADD_OPTION(ADVANCED_MODE);
    PA_ADD_OPTION(PRO_CONTROLLER1);
    PA_ADD_OPTION(LEFT_JOYCON0);
    PA_ADD_OPTION(RIGHT_JOYCON0);
    ADVANCED_MODE.add_listener(*this);
}


void KeyboardMappingOption::load_json(const JsonValue& json){
    BatchOption::load_json(json);
    KeyboardMappingOption::on_config_value_changed(this);
}
void KeyboardMappingOption::on_config_value_changed(void* object){
    PRO_CONTROLLER1.set_advanced_mode(ADVANCED_MODE);
    LEFT_JOYCON0.set_advanced_mode(ADVANCED_MODE);
    RIGHT_JOYCON0.set_advanced_mode(ADVANCED_MODE);
}












}
}
