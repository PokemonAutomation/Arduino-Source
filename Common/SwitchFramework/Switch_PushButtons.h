/*  Pokemon Automation Push Button Framework
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      This is (will be) the public API. Try to use only these functions.
 * 
 */

#ifndef PokemonAutomation_PushButtons_H
#define PokemonAutomation_PushButtons_H

//#ifdef __AVR__
//#include "NativePrograms/SwitchFramework/Master.h"
//#endif
#include "Common/MessageProtocol.h"
#include "SwitchControllerDefs.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Shared API
#ifndef __cplusplus
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  System Functions

//  Call this at the start of every program.
void initialize_framework(uint8_t program_id);

//  Turn on/off the LEDs.
void set_leds(bool on);

//  A global clock. Returns the # of ticks that have elapsed since the start of
//  the program.
uint32_t system_clock(void);

void end_program_callback(void);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Basic Framework Operations

//  Do nothing for the specified number of ticks.
void pbf_wait(uint16_t ticks);

//  Press the specified button for "hold_ticks" ticks.
//  Then release and wait for "release_ticks" ticks.
void pbf_press_button(Button button, uint16_t hold_ticks, uint16_t release_ticks);

//  Press the specified Dpad button for "hold_ticks" ticks.
//  Then release and wait for "release_ticks" ticks.
void pbf_press_dpad(DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks);

//  Joystick Positions:
//      Neutral     x = 0x80, y = 0x80
//      Up          x = 0x80, y = 0x00
//      Down        x = 0x80, y = 0xff
//      Left        x = 0x00, y = 0x80
//      Right       x = 0xff, y = 0x80
void pbf_move_left_joystick (uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);
void pbf_move_right_joystick(uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);

//  Repeatedly press the specified button as quickly as possible for the
//  specified amount of time.
void pbf_mash_button(Button button, uint16_t ticks);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Utilities

//  Wait this many ticks and flash the LEDs.
void start_program_flash(uint16_t ticks);

//  Connect controller from grip menu and go to Switch home.
void grip_menu_connect_go_home(void);

//  Keep the Switch from sleeping after the program finishes.
//  If the Switch sleeps, it may end up restarting the program!
void end_program_loop(void);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Client Side Extensions
#ifdef __cplusplus
namespace PokemonAutomation{
    class BotBaseContext;

//    //  Global instance
//    extern BotBaseContext* global_connection;

    void set_leds                   (const BotBaseContext& context, bool on);
    void end_program_callback       (const BotBaseContext& context);

    uint32_t system_clock           (const BotBaseContext& context);
    void pbf_wait                   (const BotBaseContext& context, uint16_t ticks);
    void pbf_press_button           (const BotBaseContext& context, Button button, uint16_t hold_ticks, uint16_t release_ticks);
    void pbf_press_dpad             (const BotBaseContext& context, DpadPosition position, uint16_t hold_ticks, uint16_t release_ticks);
    void pbf_move_left_joystick     (const BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);
    void pbf_move_right_joystick    (const BotBaseContext& context, uint8_t x, uint8_t y, uint16_t hold_ticks, uint16_t release_ticks);
    void pbf_mash_button            (const BotBaseContext& context, Button button, uint16_t ticks);

    void start_program_flash        (const BotBaseContext& context, uint16_t ticks);
    void grip_menu_connect_go_home  (const BotBaseContext& context);
    void end_program_loop           (const BotBaseContext& context);
}
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Message Protocols
#if _WIN32
#pragma pack(push, 1)
#define PABB_PACK
#else
#define PABB_PACK   __attribute__((packed))
#endif


#define PABB_MSG_COMMAND_PBF_WAIT                   0x90
typedef struct{
    seqnum_t seqnum;
    uint16_t ticks;
} PABB_PACK pabb_pbf_wait;

#define PABB_MSG_COMMAND_PBF_PRESS_BUTTON           0x91
typedef struct{
    seqnum_t seqnum;
    Button button;
    uint16_t hold_ticks;
    uint16_t release_ticks;
} PABB_PACK pabb_pbf_press_button;

#define PABB_MSG_COMMAND_PBF_PRESS_DPAD             0x92
typedef struct{
    seqnum_t seqnum;
    DpadPosition dpad;
    uint16_t hold_ticks;
    uint16_t release_ticks;
} PABB_PACK pabb_pbf_press_dpad;

#define PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_L        0x93
#define PABB_MSG_COMMAND_PBF_MOVE_JOYSTICK_R        0x94
typedef struct{
    seqnum_t seqnum;
    uint8_t x;
    uint8_t y;
    uint16_t hold_ticks;
    uint16_t release_ticks;
} PABB_PACK pabb_pbf_move_joystick;

#define PABB_MSG_COMMAND_MASH_BUTTON                0x95
typedef struct{
    seqnum_t seqnum;
    Button button;
    uint16_t ticks;
} PABB_PACK pabb_pbf_mash_button;


#if _WIN32
#pragma pack(pop)
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif
