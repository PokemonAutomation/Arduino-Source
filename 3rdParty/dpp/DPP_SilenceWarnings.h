/*  Silence Warnings for DPP Headers
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Include this before every DPP header to silence warnings in those headers.
 *
 */

#ifndef PokemonAutomation_3rdParty_DPP_SilenceWarnings_H
#define PokemonAutomation_3rdParty_DPP_SilenceWarnings_H


#if 0
#elif _MSC_VER

#pragma warning(disable:4100)   //  Unreferenced Formal Parameter
#pragma warning(disable:4251)   //  dll-interface

#elif __GNUC__

#pragma GCC diagnostic ignored "-Wunused-parameter"

#endif


#endif
