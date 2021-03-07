/*  Framework Settings
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "SwitchControllerDefs.h"
#include "FrameworkSettings.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  General Options

//  The initial wait period before the program does anything. This gives you
//  time to switch the Arduino from computer to Switch if connected over a KVM.
uint16_t CONNECT_CONTROLLER_DELAY         =   5 * TICKS_PER_SECOND;

//  Delay from pressing home anywhere in the settings to return to the home menu.
uint16_t SETTINGS_TO_HOME_DELAY           =   120;

//  Set this to true if starting the game requires checking the internet.
//  Otherwise, programs that require soft-resetting may not work properly.
//
//  If the game is not a physical cartridge and the Switch is not the primary
//  Switch, starting the game will require checking the internet to see if it
//  can be played. If this is the case, set this to true.
//
//  Setting this option to true will slow down soft-resetting by about 3 seconds.
bool START_GAME_REQUIRES_INTERNET       =   false;

//  Some programs can bypass the system update menu at little performance cost.
//  Setting this to true enables this.
bool TOLERATE_SYSTEM_UPDATE_MENU_FAST   =   true;

//  Some programs can bypass the system update menu, but will take a noticeable
//  performance hit. Setting this to true enables this.
bool TOLERATE_SYSTEM_UPDATE_MENU_SLOW   =   false;

