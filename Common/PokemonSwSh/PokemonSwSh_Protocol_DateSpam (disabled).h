/*  Date Spamming Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_PokemonSwSh_Protocol_DateSpam_H
#define PokemonAutomation_PokemonSwSh_Protocol_DateSpam_H

#ifdef __AVR__
#include "NativePrograms/NintendoSwitch/Framework/Master.h"
#endif
#include "Common/Microcontroller/MessageProtocol.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Protocols
#if _WIN32
#pragma pack(push, 1)
#define PABB_PACK
#else
#define PABB_PACK   __attribute__((packed))
#endif
////////////////////////////////////////////////////////////////////////////////

#define PABB_MSG_COMMAND_HOME_TO_DATE_TIME                      0xb4
typedef struct{
    seqnum_t seqnum;
    bool to_date_change;
    bool fast;
} PABB_PACK pabb_home_to_date_time;

#define PABB_MSG_COMMAND_NEUTRAL_DATE_SKIP                      0xb5
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_neutral_date_skip;

#define PABB_MSG_COMMAND_ROLL_DATE_FORWARD_1                    0xb6
typedef struct{
    seqnum_t seqnum;
    bool fast;
} PABB_PACK pabb_roll_date_forward_1;

#define PABB_MSG_COMMAND_ROLL_DATE_BACKWARD_N                   0xb7
typedef struct{
    seqnum_t seqnum;
    uint8_t skips;
    bool fast;
} PABB_PACK pabb_roll_date_backward_N;

#define PABB_MSG_COMMAND_HOME_ROLL_DATE_ENTER_GAME              0xb8
typedef struct{
    seqnum_t seqnum;
    bool rollback_year;
} PABB_PACK pabb_home_roll_date_enter_game;

#define PABB_MSG_COMMAND_TOUCH_DATE_FROM_HOME                   0xb9
typedef struct{
    seqnum_t seqnum;
    uint16_t settings_to_home_delay;
} PABB_PACK pabb_touch_date_from_home;

#define PABB_MSG_COMMAND_ROLLBACK_HOURS_FROM_HOME               0xba
typedef struct{
    seqnum_t seqnum;
    uint8_t hours;
    uint16_t settings_to_home_delay;
} PABB_PACK pabb_rollback_hours_from_home;

////////////////////////////////////////////////////////////////////////////////
#if _WIN32
#pragma pack(pop)
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif

