/*  Date Spamming Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_PokemonSwShDateSpam_H
#define PokemonAutomation_PokemonSwShDateSpam_H

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
#define MAX_YEAR    60

#ifdef __cplusplus

//  From the Switch Home (cursor over 1st game), navigate all the way to the
//  date change button.
//  If (fast = true), it will run slightly faster, but has a higher chance of
//  not making all way in. Do this only if the program is able to self-recover.
void home_to_date_time(bool fast);

//  Call this immediately after calling "home_to_date_time()".
//  This function will roll the 1st and 3rd slots forward by one.
void roll_date_forward_1(bool fast);

//  Call this immediately after calling "home_to_date_time()".
//  This function will roll the 1st and 3rd slots backwards by N.
void roll_date_backward_N(uint8_t skips, bool fast);

//  From the Switch Home (cursor over 1st game), roll the date forward and
//  re-enter the game.
//
//  This function does not wait after the final press. Recommend waiting 100 ticks
//  before you are safely in the game.
//
//  The first time you call this function, it will roll the year back to 2000.
//  Then every 60 calls, it will roll it back to 2000 again. It automatically
//  keeps track of this count.
//
void home_roll_date_enter_game_autorollback(uint8_t* year);

//  From the Switch Home (cursor over 1st game), roll the date forward and
//  re-enter the game.
//
//  This function does not wait after the final press. Recommend waiting 100 ticks
//  before you are safely in the game.
//
//  This function does not automatically roll back the year to 2000. You tell it
//  whether to do roll back with the "rollback_year" parameter.
//
void home_roll_date_enter_game(bool rollback_year);

//  Touch the date without changing it. This prevents unintentional rollovers.
//  Start this function in the Switch home with the icon over the first game.
//  When the function finishes, it will be back in the same position.
void touch_date_from_home(uint16_t settings_to_home_delay);

//  Roll back by this many hours.
//  Start this function in the Switch home with the icon over the first game.
//  When the function finishes, it will be back in the same position.
void rollback_hours_from_home(uint8_t hours, uint16_t settings_to_home_delay);

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
void home_to_date_time                      (PokemonAutomation::BotBase& device, bool fast);
void roll_date_forward_1                    (PokemonAutomation::BotBase& device, bool fast);
void roll_date_backward_N                   (PokemonAutomation::BotBase& device, uint8_t skips, bool fast);
void home_roll_date_enter_game              (PokemonAutomation::BotBase& device, bool rollback_year);
void home_roll_date_enter_game_autorollback (PokemonAutomation::BotBase& device, uint8_t* year);
void touch_date_from_home                   (PokemonAutomation::BotBase& device, uint16_t settings_to_home_delay);
void rollback_hours_from_home               (PokemonAutomation::BotBase& device, uint8_t hours, uint16_t settings_to_home_delay);
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

#define PABB_MSG_COMMAND_HOME_TO_DATE_TIME                      0xb4
typedef struct{
    seqnum_t seqnum;
    bool fast;
} PABB_PACK pabb_home_to_date_time;

#define PABB_MSG_COMMAND_ROLL_DATE_FORWARD_1                    0xb5
typedef struct{
    seqnum_t seqnum;
    bool fast;
} PABB_PACK pabb_roll_date_forward_1;

#define PABB_MSG_COMMAND_ROLL_DATE_BACKWARD_N                   0xb6
typedef struct{
    seqnum_t seqnum;
    uint8_t skips;
    bool fast;
} PABB_PACK pabb_roll_date_backward_N;

#define PABB_MSG_COMMAND_HOME_ROLL_DATE_ENTER_GAME              0xb7
typedef struct{
    seqnum_t seqnum;
    bool rollback_year;
} PABB_PACK pabb_home_roll_date_enter_game;

#define PABB_MSG_COMMAND_TOUCH_DATE_FROM_HOME                   0xb8
typedef struct{
    seqnum_t seqnum;
    uint16_t settings_to_home_delay;
} PABB_PACK pabb_touch_date_from_home;

#define PABB_MSG_COMMAND_ROLLBACK_HOURS_FROM_HOME               0xb9
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

