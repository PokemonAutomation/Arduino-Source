/*  Serial Connection
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_SerialConnection_H
#define PokemonAutomation_SerialConnection_H

#ifdef _WIN32
#include "SerialConnectionWinAPI.h"
#else
#include "SerialConnectionPOSIX.h"
#endif

#endif
