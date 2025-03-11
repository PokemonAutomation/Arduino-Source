/*  Egg Routines
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 *      This file requires (PABB_PABOTBASE_LEVEL >= 31).
 * 
 */

#ifndef PokemonAutomation_PokemonSwSh_Protocol_EggRoutines_H
#define PokemonAutomation_PokemonSwSh_Protocol_EggRoutines_H

#ifdef __AVR__
#include "NativePrograms/NintendoSwitch/Framework/Master.h"
#endif
#include "Common/Microcontroller/MessageProtocol.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"

#define TRAVEL_TO_SPIN_SPOT_DURATION    (300)
#define TRAVEL_BACK_TO_LADY_DURATION    (30 + 260 + (620) + 120 + 120 * 0)

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

#define PABB_MSG_COMMAND_EGG_FETCHER_LOOP                       0xc5
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_eggfetcher_loop;

#define PABB_MSG_COMMAND_MOVE_WHILE_MASHING_B                   0xc6
typedef struct{
    seqnum_t seqnum;
    uint16_t duration;
} PABB_PACK pabb_move_while_mashing_B;

#define PABB_MSG_COMMAND_SPIN_AND_MASH_A                        0xc7
typedef struct{
    seqnum_t seqnum;
    uint16_t duration;
} PABB_PACK pabb_spin_and_mash_A;

#define PABB_MSG_COMMAND_TRAVEL_TO_SPIN_LOCATION                0xc8
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_travel_to_spin_location;

#define PABB_MSG_COMMAND_TRAVEL_BACK_TO_LADY                    0xc9
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_travel_back_to_lady;

////////////////////////////////////////////////////////////////////////////////
#if _WIN32
#pragma pack(pop)
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif

