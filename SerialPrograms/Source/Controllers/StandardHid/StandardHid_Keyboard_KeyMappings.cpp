/*  HID Keyboard Mappings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "StandardHid_Keyboard_KeyMappings.h"

namespace PokemonAutomation{
namespace StandardHid{



const std::map<QtKeyMap::QtKey, KeyboardKey>& KEYID_TO_HID_QWERTY(){
    static const std::map<QtKeyMap::QtKey, KeyboardKey> database{
        {Qt::Key::Key_A,    KeyboardKey::KEY_A},
        {Qt::Key::Key_B,    KeyboardKey::KEY_B},
        {Qt::Key::Key_C,    KeyboardKey::KEY_C},
        {Qt::Key::Key_D,    KeyboardKey::KEY_D},
        {Qt::Key::Key_E,    KeyboardKey::KEY_E},
        {Qt::Key::Key_F,    KeyboardKey::KEY_F},
        {Qt::Key::Key_G,    KeyboardKey::KEY_G},
        {Qt::Key::Key_H,    KeyboardKey::KEY_H},
        {Qt::Key::Key_I,    KeyboardKey::KEY_I},
        {Qt::Key::Key_J,    KeyboardKey::KEY_J},
        {Qt::Key::Key_K,    KeyboardKey::KEY_K},
        {Qt::Key::Key_L,    KeyboardKey::KEY_L},
        {Qt::Key::Key_M,    KeyboardKey::KEY_M},
        {Qt::Key::Key_N,    KeyboardKey::KEY_N},
        {Qt::Key::Key_O,    KeyboardKey::KEY_O},
        {Qt::Key::Key_P,    KeyboardKey::KEY_P},
        {Qt::Key::Key_Q,    KeyboardKey::KEY_Q},
        {Qt::Key::Key_R,    KeyboardKey::KEY_R},
        {Qt::Key::Key_S,    KeyboardKey::KEY_S},
        {Qt::Key::Key_T,    KeyboardKey::KEY_T},
        {Qt::Key::Key_U,    KeyboardKey::KEY_U},
        {Qt::Key::Key_V,    KeyboardKey::KEY_V},
        {Qt::Key::Key_W,    KeyboardKey::KEY_W},
        {Qt::Key::Key_X,    KeyboardKey::KEY_X},
        {Qt::Key::Key_Y,    KeyboardKey::KEY_Y},
        {Qt::Key::Key_Z,    KeyboardKey::KEY_Z},

        {Qt::Key::Key_1,    KeyboardKey::KEY_1},
        {Qt::Key::Key_2,    KeyboardKey::KEY_2},
        {Qt::Key::Key_3,    KeyboardKey::KEY_3},
        {Qt::Key::Key_4,    KeyboardKey::KEY_4},
        {Qt::Key::Key_5,    KeyboardKey::KEY_5},
        {Qt::Key::Key_6,    KeyboardKey::KEY_6},
        {Qt::Key::Key_7,    KeyboardKey::KEY_7},
        {Qt::Key::Key_8,    KeyboardKey::KEY_8},
        {Qt::Key::Key_9,    KeyboardKey::KEY_9},
        {Qt::Key::Key_0,    KeyboardKey::KEY_0},

        {Qt::Key::Key_Enter,        KeyboardKey::KEY_ENTER},
        {Qt::Key::Key_Escape,       KeyboardKey::KEY_ESC},
        {Qt::Key::Key_Backspace,    KeyboardKey::KEY_BACKSPACE},
        {Qt::Key::Key_Tab,          KeyboardKey::KEY_TAB},
        {Qt::Key::Key_Space,        KeyboardKey::KEY_SPACE},
        {Qt::Key::Key_Minus,        KeyboardKey::KEY_MINUS},
        {Qt::Key::Key_Equal,        KeyboardKey::KEY_EQUAL},
        {Qt::Key::Key_BraceLeft,    KeyboardKey::KEY_LEFT_BRACE},
        {Qt::Key::Key_BracketLeft,  KeyboardKey::KEY_LEFT_BRACE},
        {Qt::Key::Key_BraceRight,   KeyboardKey::KEY_RIGHT_BRACE},
        {Qt::Key::Key_BracketRight, KeyboardKey::KEY_RIGHT_BRACE},
        {Qt::Key::Key_Backslash,    KeyboardKey::KEY_BACKSLASH},
        {Qt::Key::Key_Semicolon,    KeyboardKey::KEY_SEMICOLON},
        {Qt::Key::Key_Apostrophe,   KeyboardKey::KEY_APOSTROPHE},

        {Qt::Key::Key_AsciiTilde,   KeyboardKey::KEY_GRAVE},

        {Qt::Key::Key_Comma,        KeyboardKey::KEY_COMMA},
        {Qt::Key::Key_Period,       KeyboardKey::KEY_DOT},
        {Qt::Key::Key_Slash,        KeyboardKey::KEY_SLASH},
        {Qt::Key::Key_CapsLock,     KeyboardKey::KEY_CAPS_LOCK},

        {Qt::Key::Key_F1,           KeyboardKey::KEY_F1},
        {Qt::Key::Key_F2,           KeyboardKey::KEY_F2},
        {Qt::Key::Key_F3,           KeyboardKey::KEY_F3},
        {Qt::Key::Key_F4,           KeyboardKey::KEY_F4},
        {Qt::Key::Key_F5,           KeyboardKey::KEY_F5},
        {Qt::Key::Key_F6,           KeyboardKey::KEY_F6},
        {Qt::Key::Key_F7,           KeyboardKey::KEY_F7},
        {Qt::Key::Key_F8,           KeyboardKey::KEY_F8},
        {Qt::Key::Key_F9,           KeyboardKey::KEY_F9},
        {Qt::Key::Key_F10,          KeyboardKey::KEY_F10},
        {Qt::Key::Key_F11,          KeyboardKey::KEY_F11},
        {Qt::Key::Key_F12,          KeyboardKey::KEY_F12},

        {Qt::Key::Key_Print,        KeyboardKey::KEY_PRINT_SCREEN},
        {Qt::Key::Key_ScrollLock,   KeyboardKey::KEY_SCROLL_LOCK},
        {Qt::Key::Key_Pause,        KeyboardKey::KEY_PAUSE},
        {Qt::Key::Key_Insert,       KeyboardKey::KEY_INSERT},
        {Qt::Key::Key_Home,         KeyboardKey::KEY_HOME},
        {Qt::Key::Key_PageUp,       KeyboardKey::KEY_PAGE_UP},
        {Qt::Key::Key_Delete,       KeyboardKey::KEY_DELETE},
        {Qt::Key::Key_End,          KeyboardKey::KEY_END},
        {Qt::Key::Key_PageDown,     KeyboardKey::KEY_PAGE_DOWN},
        {Qt::Key::Key_Right,        KeyboardKey::KEY_RIGHT},
        {Qt::Key::Key_Left,         KeyboardKey::KEY_LEFT},
        {Qt::Key::Key_Down,         KeyboardKey::KEY_DOWN},
        {Qt::Key::Key_Up,           KeyboardKey::KEY_UP},
        {Qt::Key::Key_NumLock,      KeyboardKey::KEY_NUM_LOCK},

        {{Qt::Key::Key_Slash,       true},  KeyboardKey::KEY_KP_SLASH},
        {{Qt::Key::Key_Asterisk,    true},  KeyboardKey::KEY_KP_ASTERISK},
        {{Qt::Key::Key_Minus,       true},  KeyboardKey::KEY_KP_MINUS},
        {{Qt::Key::Key_Plus,        true},  KeyboardKey::KEY_KP_PLUS},
        {{Qt::Key::Key_Enter,       true},  KeyboardKey::KEY_KP_ENTER},
        {{Qt::Key::Key_1,           true},  KeyboardKey::KEY_KP_1},
        {{Qt::Key::Key_2,           true},  KeyboardKey::KEY_KP_2},
        {{Qt::Key::Key_3,           true},  KeyboardKey::KEY_KP_3},
        {{Qt::Key::Key_4,           true},  KeyboardKey::KEY_KP_4},
        {{Qt::Key::Key_5,           true},  KeyboardKey::KEY_KP_5},
        {{Qt::Key::Key_6,           true},  KeyboardKey::KEY_KP_6},
        {{Qt::Key::Key_7,           true},  KeyboardKey::KEY_KP_7},
        {{Qt::Key::Key_8,           true},  KeyboardKey::KEY_KP_8},
        {{Qt::Key::Key_9,           true},  KeyboardKey::KEY_KP_9},
        {{Qt::Key::Key_0,           true},  KeyboardKey::KEY_KP_0},
        {{Qt::Key::Key_Period,      true},  KeyboardKey::KEY_KP_DOT},

        {Qt::Key::Key_F13,          KeyboardKey::KEY_F13},
        {Qt::Key::Key_F14,          KeyboardKey::KEY_F14},
        {Qt::Key::Key_F15,          KeyboardKey::KEY_F15},
        {Qt::Key::Key_F16,          KeyboardKey::KEY_F16},
        {Qt::Key::Key_F17,          KeyboardKey::KEY_F17},
        {Qt::Key::Key_F18,          KeyboardKey::KEY_F18},
        {Qt::Key::Key_F19,          KeyboardKey::KEY_F19},
        {Qt::Key::Key_F20,          KeyboardKey::KEY_F20},
        {Qt::Key::Key_F21,          KeyboardKey::KEY_F21},
        {Qt::Key::Key_F22,          KeyboardKey::KEY_F22},
        {Qt::Key::Key_F23,          KeyboardKey::KEY_F23},
        {Qt::Key::Key_F24,          KeyboardKey::KEY_F24},

        {Qt::Key::Key_Open,         KeyboardKey::KEY_OPEN},
        {Qt::Key::Key_Help,         KeyboardKey::KEY_HELP},
        {Qt::Key::Key_Menu,         KeyboardKey::KEY_MENU},
        {Qt::Key::Key_Select,       KeyboardKey::KEY_SELECT},
        {Qt::Key::Key_Stop,         KeyboardKey::KEY_STOP},
//        {Qt::Key::Key_Again,        KeyboardKey::KEY_AGAIN},
        {Qt::Key::Key_Undo,         KeyboardKey::KEY_UNDO},
        {Qt::Key::Key_Cut,          KeyboardKey::KEY_CUT},
        {Qt::Key::Key_Copy,         KeyboardKey::KEY_COPY},
        {Qt::Key::Key_Paste,        KeyboardKey::KEY_PASTE},
        {Qt::Key::Key_Find,         KeyboardKey::KEY_FIND},
        {Qt::Key::Key_VolumeMute,   KeyboardKey::KEY_VOLUME_MUTE},
        {Qt::Key::Key_VolumeUp,     KeyboardKey::KEY_VOLUME_UP},
        {Qt::Key::Key_VolumeDown,   KeyboardKey::KEY_VOLUME_DOWN},

        {Qt::Key::Key_Control,      KeyboardKey::KEY_LEFT_CTRL},
        {Qt::Key::Key_Shift,        KeyboardKey::KEY_LEFT_SHIFT},
        {Qt::Key::Key_Alt,          KeyboardKey::KEY_LEFT_ALT},
        {Qt::Key::Key_Meta,         KeyboardKey::KEY_LEFT_META},

//        {Qt::Key::Key_MediaPlay,    KeyboardKey::KEY_MEDIA_PLAYPAUSE},
//        {Qt::Key::Key_MediaStop,    KeyboardKey::KEY_MEDIA_STOPCD},
//        {Qt::Key::Key_MediaPrevious,KeyboardKey::KEY_MEDIA_PREVIOUS_SONG},
//        {Qt::Key::Key_MediaNext,    KeyboardKey::KEY_MEDIA_NEXT_SONG},
//        {Qt::Key::Key_Eject,        KeyboardKey::KEY_MEDIA_EJECTCD},
//        {Qt::Key::Key_Eject,        KeyboardKey::KEY_MEDIA_VOLUME_UP},
//        {Qt::Key::Key_Eject,        KeyboardKey::KEY_MEDIA_VOLUME_DOWN},
    };
    return database;
};











}
}
