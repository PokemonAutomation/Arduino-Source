/*  HID Keyboard Controller Buttons
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_StandardHid_Keyboard_ControllerButtons_H
#define PokemonAutomation_StandardHid_Keyboard_ControllerButtons_H

#include <stdint.h>


namespace PokemonAutomation{
namespace StandardHid{



//  Taken from: https://gist.github.com/MightyPork/6da26e382a7ad91b5496ee55fdc73db2
enum KeyboardKey : uint8_t{
    KEY_A   =   0x04,
    KEY_B   =   0x05,
    KEY_C   =   0x06,
    KEY_D   =   0x07,
    KEY_E   =   0x08,
    KEY_F   =   0x09,
    KEY_G   =   0x0a,
    KEY_H   =   0x0b,
    KEY_I   =   0x0c,
    KEY_J   =   0x0d,
    KEY_K   =   0x0e,
    KEY_L   =   0x0f,
    KEY_M   =   0x10,
    KEY_N   =   0x11,
    KEY_O   =   0x12,
    KEY_P   =   0x13,
    KEY_Q   =   0x14,
    KEY_R   =   0x15,
    KEY_S   =   0x16,
    KEY_T   =   0x17,
    KEY_U   =   0x18,
    KEY_V   =   0x19,
    KEY_W   =   0x1a,
    KEY_X   =   0x1b,
    KEY_Y   =   0x1c,
    KEY_Z   =   0x1d,

    KEY_1   =   0x1e,
    KEY_2   =   0x1f,
    KEY_3   =   0x20,
    KEY_4   =   0x21,
    KEY_5   =   0x22,
    KEY_6   =   0x23,
    KEY_7   =   0x24,
    KEY_8   =   0x25,
    KEY_9   =   0x26,
    KEY_0   =   0x27,

    KEY_ENTER       =   0x28,
    KEY_ESC         =   0x29,
    KEY_BACKSPACE   =   0x2a,
    KEY_TAB         =   0x2b,
    KEY_SPACE       =   0x2c,
    KEY_MINUS       =   0x2d,   //  - and _
    KEY_EQUAL       =   0x2e,   //  = and +
    KEY_LEFT_BRACE  =   0x2f,   //  [ and {
    KEY_RIGHT_BRACE =   0x30,   //  ] and }
    KEY_BACKSLASH   =   0x31,   //  \ and |
    KEY_HASHTILDE   =   0x32,   //  # and ~ (non-US)
    KEY_SEMICOLON   =   0x33,   //  ; and :
    KEY_APOSTROPHE  =   0x34,   //  ' and "
    KEY_GRAVE       =   0x35,   //  ` and ~
    KEY_COMMA       =   0x36,   //  , and <
    KEY_DOT         =   0x37,   //  . and >
    KEY_SLASH       =   0x38,   //  / and ?
    KEY_CAPS_LOCK   =   0x39,

    KEY_F1          =   0x3a,
    KEY_F2          =   0x3b,
    KEY_F3          =   0x3c,
    KEY_F4          =   0x3d,
    KEY_F5          =   0x3e,
    KEY_F6          =   0x3f,
    KEY_F7          =   0x40,
    KEY_F8          =   0x41,
    KEY_F9          =   0x42,
    KEY_F10         =   0x43,
    KEY_F11         =   0x44,
    KEY_F12         =   0x45,

    KEY_PRINT_SCREEN    =   0x46,
    KEY_SCROLL_LOCK     =   0x47,
    KEY_PAUSE           =   0x48,
    KEY_INSERT          =   0x49,
    KEY_HOME            =   0x4a,
    KEY_PAGE_UP         =   0x4b,
    KEY_DELETE          =   0x4c,
    KEY_END             =   0x4d,
    KEY_PAGE_DOWN       =   0x4e,
    KEY_RIGHT           =   0x4f,
    KEY_LEFT            =   0x50,
    KEY_DOWN            =   0x51,
    KEY_UP              =   0x52,
    KEY_NUM_LOCK        =   0x53,
    KEY_KP_SLASH        =   0x54,
    KEY_KP_ASTERISK     =   0x55,
    KEY_KP_MINUS        =   0x56,
    KEY_KP_PLUS         =   0x57,
    KEY_KP_ENTER        =   0x58,
    KEY_KP_1            =   0x59,
    KEY_KP_2            =   0x5a,
    KEY_KP_3            =   0x5b,
    KEY_KP_4            =   0x5c,
    KEY_KP_5            =   0x5d,
    KEY_KP_6            =   0x5e,
    KEY_KP_7            =   0x5f,
    KEY_KP_8            =   0x60,
    KEY_KP_9            =   0x61,
    KEY_KP_0            =   0x62,
    KEY_KP_DOT          =   0x63,   //  . and Del

    KEY_102ND           =   0x64,   //  \ and | (non-US)
    KEY_COMPOSE         =   0x65,
    KEY_POWER           =   0x66,
    KEY_KP_EQUAL        =   0x67,

    KEY_F13         =   0x68,
    KEY_F14         =   0x69,
    KEY_F15         =   0x6a,
    KEY_F16         =   0x6b,
    KEY_F17         =   0x6c,
    KEY_F18         =   0x6d,
    KEY_F19         =   0x6e,
    KEY_F20         =   0x6f,
    KEY_F21         =   0x70,
    KEY_F22         =   0x71,
    KEY_F23         =   0x72,
    KEY_F24         =   0x73,

    KEY_OPEN        =   0x74,
    KEY_HELP        =   0x75,
    KEY_MENU        =   0x76,
    KEY_SELECT      =   0x77,
    KEY_STOP        =   0x78,
    KEY_AGAIN       =   0x79,
    KEY_UNDO        =   0x7a,
    KEY_CUT         =   0x7b,
    KEY_COPY        =   0x7c,
    KEY_PASTE       =   0x7d,
    KEY_FIND        =   0x7e,

    KEY_VOLUME_MUTE =   0x7f,
    KEY_VOLUME_UP   =   0x80,
    KEY_VOLUME_DOWN =   0x81,

    KEY_KP_COMMA    =   0x85,

    KEY_LEFT_CTRL   =   0xe0,
    KEY_LEFT_SHIFT  =   0xe1,
    KEY_LEFT_ALT    =   0xe2,
    KEY_LEFT_META   =   0xe3,
    KEY_RIGHT_CTRL  =   0xe4,
    KEY_RIGHT_SHIFT =   0xe5,
    KEY_RIGHT_ALT   =   0xe6,
    KEY_RIGHT_META  =   0xe7,

    KEY_MEDIA_PLAYPAUSE     =   0xe8,
    KEY_MEDIA_STOPCD        =   0xe9,
    KEY_MEDIA_PREVIOUS_SONG =   0xea,
    KEY_MEDIA_NEXT_SONG     =   0xeb,
    KEY_MEDIA_EJECTCD       =   0xec,
    KEY_MEDIA_VOLUME_UP     =   0xed,
    KEY_MEDIA_VOLUME_DOWN   =   0xee,
    KEY_MEDIA_MUTE          =   0xef,
    KEY_MEDIA_WWW           =   0xf0,
    KEY_MEDIA_BACK          =   0xf1,
    KEY_MEDIA_FORWARD       =   0xf2,
    KEY_MEDIA_STOP          =   0xf3,
    KEY_MEDIA_FIND          =   0xf4,
    KEY_MEDIA_SCROLL_UP     =   0xf5,
    KEY_MEDIA_SCROLL_DOWN   =   0xf6,
    KEY_MEDIA_EDIT          =   0xf7,
    KEY_MEDIA_SLEEP         =   0xf8,
    KEY_MEDIA_COFFEE        =   0xf9,
    KEY_MEDIA_REFRESH       =   0xfa,
    KEY_MEDIA_CALC          =   0xfb,
};








}
}
#endif
