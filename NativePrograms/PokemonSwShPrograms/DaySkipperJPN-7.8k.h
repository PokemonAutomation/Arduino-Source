/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 */

#include "Common/SwitchFramework/SwitchControllerDefs.h"


//  The maximum number of skips to perform.
//  The actual # of skips will be lower if any errors are made.
extern const uint32_t SKIPS;

//  If you don't want to start on 1/1/2000, you can set this instead.
typedef struct{
    uint16_t year;
    uint8_t month;
    uint8_t day;
} Date;
extern const Date START_DATE;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Advanced Settings (you shouldn't need to change these)

//  Run auto-recovery every this # of skips. Zero disables the auto-corrections.
//  At 500, the overhead is approximately 0.5%.
extern const uint16_t CORRECTION_SKIPS;

