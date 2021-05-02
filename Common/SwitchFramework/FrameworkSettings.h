/*  Framework Settings
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#ifndef PokemonAutomation_FrameworkSettings_H
#define PokemonAutomation_FrameworkSettings_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
#include <string>
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  General Options

//  The initial wait period before the program does anything. This gives you
//  time to switch the Arduino from computer to Switch if connected over a KVM.
extern uint16_t CONNECT_CONTROLLER_DELAY;

//  Delay from pressing home anywhere in the settings to return to the home menu.
extern uint16_t SETTINGS_TO_HOME_DELAY;

//  Set this to true if starting the game requires checking the internet.
//  Otherwise, programs that require soft-resetting may not work properly.
//
//  If the game is not a physical cartridge and the Switch is not the primary
//  Switch, starting the game will require checking the internet to see if it
//  can be played. If this is the case, set this to true.
//
//  Setting this option to true will slow down soft-resetting by about 3 seconds.
extern bool START_GAME_REQUIRES_INTERNET;

//  Some programs can bypass the system update menu at little performance cost.
//  Setting this to true enables this.
extern bool TOLERATE_SYSTEM_UPDATE_MENU_FAST;

//  Some programs have the ability to tolerate the system update menu at the
//  cost of speed/performance. Setting this to true enables this.
extern bool TOLERATE_SYSTEM_UPDATE_MENU_SLOW;

#ifdef __cplusplus

//  Some programs can send discord messages in your own private server. Set this
//  to your discord webhook ID.
extern std::string DISCORD_WEBHOOK_ID;

//  Some programs can send discord messages in your own private server. Set this
//  to your discord webhook token.
extern std::string DISCORD_WEBHOOK_TOKEN;

//  Some programs can send discord messages in your own private server. Set this
//  to your discord user ID.
extern std::string DISCORD_USER_ID;

//  Some programs can send discord messages in your own private server. Set this
//  to your discord user short name.
extern std::string DISCORD_USER_SHORT_NAME;

#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#endif
