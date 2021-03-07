/*  Day Skippers
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 *      This file requires (PABB_PABOTBASE_LEVEL >= 31).
 * 
 */

#ifndef PokemonAutomation_PokemonSwShDaySkippers_H
#define PokemonAutomation_PokemonSwShDaySkippers_H

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
void skipper_init_view              (void);
void skipper_auto_recovery          (void);
void skipper_rollback_year_full     (bool date_us);
void skipper_rollback_year_sync     (void);
void skipper_increment_day          (bool date_us);
void skipper_increment_month        (uint8_t days);
void skipper_increment_all          (void);
void skipper_increment_all_rollback (void);
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
void skipper_init_view              (PokemonAutomation::BotBase& device);
void skipper_auto_recovery          (PokemonAutomation::BotBase& device);
void skipper_rollback_year_full     (PokemonAutomation::BotBase& device, bool date_us);
void skipper_rollback_year_sync     (PokemonAutomation::BotBase& device);
void skipper_increment_day          (PokemonAutomation::BotBase& device, bool date_us);
void skipper_increment_month        (PokemonAutomation::BotBase& device, uint8_t days);
void skipper_increment_all          (PokemonAutomation::BotBase& device);
void skipper_increment_all_rollback (PokemonAutomation::BotBase& device);
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

#define PABB_MSG_COMMAND_SKIPPER_INIT_VIEW                      0xbe
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_skipper_init_view;

#define PABB_MSG_COMMAND_SKIPPER_AUTO_RECOVERY                  0xbf
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_skipper_auto_recovery;

#define PABB_MSG_COMMAND_SKIPPER_ROLLBACK_YEAR_FULL             0xc0
typedef struct{
    seqnum_t seqnum;
    bool date_us;
} PABB_PACK pabb_skipper_rollback_year_full;

#define PABB_MSG_COMMAND_SKIPPER_ROLLBACK_YEAR_SYNC             0xc1
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_skipper_rollback_year_sync;

#define PABB_MSG_COMMAND_SKIPPER_INCREMENT_DAY                  0xc2
typedef struct{
    seqnum_t seqnum;
    bool date_us;
} PABB_PACK pabb_skipper_increment_day;

#define PABB_MSG_COMMAND_SKIPPER_INCREMENT_MONTH                0xc3
typedef struct{
    seqnum_t seqnum;
    uint8_t days;
} PABB_PACK pabb_skipper_increment_month;

#define PABB_MSG_COMMAND_SKIPPER_INCREMENT_ALL                  0xc4
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_skipper_increment_all;

#define PABB_MSG_COMMAND_SKIPPER_INCREMENT_ALL_ROLLBACK         0xc5
typedef struct{
    seqnum_t seqnum;
} PABB_PACK pabb_skipper_increment_all_rollback;

////////////////////////////////////////////////////////////////////////////////
#if _WIN32
#pragma pack(pop)
#endif
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif

