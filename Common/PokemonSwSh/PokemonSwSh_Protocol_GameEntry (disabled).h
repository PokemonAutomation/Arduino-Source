/*  Game Entry Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_PokemonSwSh_Protocol_GameEntry_H
#define PokemonAutomation_PokemonSwSh_Protocol_GameEntry_H

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

#define PABB_MSG_COMMAND_SETTINGS_TO_ENTER_GAME_DEN_LOBBY       0xb0
typedef struct{
    seqnum_t seqnum;
    bool tolerate_update_menu;
    bool fast;
    uint16_t enter_switch_pokemon_delay;
    uint16_t exit_switch_pokemon_delay;
} PABB_PACK pabb_settings_to_enter_game_den_lobby;

#define PABB_MSG_COMMAND_ENTER_GAME                             0xb1
typedef struct{
    seqnum_t seqnum;
    bool backup_save;
    uint16_t enter_game_mash;
    uint16_t enter_game_wait;
} PABB_PACK pabb_enter_game;

#define PABB_MSG_COMMAND_FAST_RESET_GAME                        0xb3
typedef struct{
    seqnum_t seqnum;
    uint16_t start_game_mash;
    uint16_t start_game_wait;
    uint16_t enter_game_mash;
    uint16_t enter_game_wait;
} PABB_PACK pabb_fast_reset_game;

////////////////////////////////////////////////////////////////////////////////
#if _WIN32
#pragma pack(pop)
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif

