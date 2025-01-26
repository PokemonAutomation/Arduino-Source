/*  Day Skippers
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 *      This file requires (PABB_PABOTBASE_LEVEL >= 31).
 * 
 */

#ifndef PokemonAutomation_PokemonSwSh_Protocol_DaySkippers_H
#define PokemonAutomation_PokemonSwSh_Protocol_DaySkippers_H

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
#ifdef __cplusplus
namespace PokemonAutomation{
namespace NintendoSwitch{
#endif
////////////////////////////////////////////////////////////////////////////////

#define PABB_MSG_COMMAND_SKIPPER_INIT_VIEW                      0xbd
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_skipper_init_view;

#define PABB_MSG_COMMAND_SKIPPER_AUTO_RECOVERY                  0xbe
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_skipper_auto_recovery;

#define PABB_MSG_COMMAND_SKIPPER_ROLLBACK_YEAR_FULL             0xbf
typedef struct{
    seqnum_t seqnum;
    bool date_us;
} PABB_PACK pabb_skipper_rollback_year_full;

#define PABB_MSG_COMMAND_SKIPPER_ROLLBACK_YEAR_SYNC             0xc0
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_skipper_rollback_year_sync;

#define PABB_MSG_COMMAND_SKIPPER_INCREMENT_DAY                  0xc1
typedef struct{
    seqnum_t seqnum;
    bool date_us;
} PABB_PACK pabb_skipper_increment_day;

#define PABB_MSG_COMMAND_SKIPPER_INCREMENT_MONTH                0xc2
typedef struct{
    seqnum_t seqnum;
    uint8_t days;
} PABB_PACK pabb_skipper_increment_month;

#define PABB_MSG_COMMAND_SKIPPER_INCREMENT_ALL                  0xc3
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_skipper_increment_all;

#define PABB_MSG_COMMAND_SKIPPER_INCREMENT_ALL_ROLLBACK         0xc4
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_skipper_increment_all_rollback;

////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
}
#endif
#if _WIN32
#pragma pack(pop)
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif

