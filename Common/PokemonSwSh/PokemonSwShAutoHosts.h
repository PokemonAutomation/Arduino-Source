/*  Auto Host Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_PokemonSwShAutoHosts_H
#define PokemonAutomation_PokemonSwShAutoHosts_H

#ifdef __AVR__
#include "NativePrograms/SwitchFramework/Master.h"
#endif
#include "Common/MessageProtocol.h"
#include "Common/SwitchFramework/SwitchControllerDefs.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Universal
#ifdef __cplusplus
void connect_to_internet(uint16_t open_ycomm_delay, uint16_t connect_to_internet_delay);
void home_to_add_friends(uint8_t user_slot, bool fix_cursor);
uint16_t accept_FRs(
    uint8_t slot, bool fix_cursor,
    uint16_t game_to_home_delay_safe,
    uint16_t auto_fr_duration,
    bool tolerate_system_update_window_slow
);
void accept_FRs_while_waiting(
    uint8_t slot, uint16_t wait_time,
    uint16_t game_to_home_delay_safe,
    uint16_t auto_fr_duration,
    bool tolerate_system_update_window_slow
);
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Client Side
#ifdef __cplusplus
namespace PokemonAutomation{
    class BotBase;
}
void connect_to_internet(PokemonAutomation::BotBase& device, uint16_t open_ycomm_delay, uint16_t connect_to_internet_delay);
void home_to_add_friends(PokemonAutomation::BotBase& device, uint8_t user_slot, bool fix_cursor);
uint16_t accept_FRs(
    PokemonAutomation::BotBase& device,
    uint8_t slot, bool fix_cursor,
    uint16_t game_to_home_delay_safe,
    uint16_t auto_fr_duration,
    bool tolerate_system_update_window_slow
);
void accept_FRs_while_waiting(
    PokemonAutomation::BotBase& device,
    uint8_t slot, uint16_t wait_time,
    uint16_t game_to_home_delay_safe,
    uint16_t auto_fr_duration,
    bool tolerate_system_update_window_slow
);
#endif
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

#define PABB_MSG_COMMAND_CONNECT_TO_INTERNET                    0xba
typedef struct{
    seqnum_t seqnum;
    uint16_t open_ycomm_delay;
    uint16_t connect_to_internet_delay;
} PABB_PACK pabb_connect_to_internet;

#define PABB_MSG_COMMAND_HOME_TO_ADD_FRIENDS                    0xbb
typedef struct{
    seqnum_t seqnum;
    uint8_t user_slot;
    bool fix_cursor;
} PABB_PACK pabb_home_to_add_friends;

#define PABB_MSG_COMMAND_ACCEPT_FRS                             0xbc
typedef struct{
    seqnum_t seqnum;
    uint8_t slot;
    bool fix_cursor;
    uint16_t game_to_home_delay_safe;
    uint16_t auto_fr_duration;
    bool tolerate_system_update_window_slow;
} PABB_PACK pabb_accept_FRs;

#define PABB_MSG_COMMAND_ACCEPT_FRS_WHILE_WAITING               0xbd
typedef struct{
    seqnum_t seqnum;
    uint8_t slot;
    uint16_t wait_time;
    uint16_t game_to_home_delay_safe;
    uint16_t auto_fr_duration;
    bool tolerate_system_update_window_slow;
} PABB_PACK pabb_accept_FRs_while_waiting;

////////////////////////////////////////////////////////////////////////////////
#if _WIN32
#pragma pack(pop)
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif

