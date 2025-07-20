/*  Environment (Windows)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Environment_Windows_H
#define PokemonAutomation_Environment_Windows_H

namespace PokemonAutomation{


enum class ThreadPriority{
    Realtime,
    High,
    AboveNormal,
    Normal,
    BelowNormal,
    Low,
};

constexpr ThreadPriority DEFAULT_PRIORITY_REALTIME              = ThreadPriority::High;
constexpr ThreadPriority DEFAULT_PRIORITY_REALTIME_INFERENCE    = ThreadPriority::AboveNormal;
constexpr ThreadPriority DEFAULT_PRIORITY_NORMAL_INFERENCE      = ThreadPriority::BelowNormal;
constexpr ThreadPriority DEFAULT_PRIORITY_COMPUTE               = ThreadPriority::BelowNormal;










}
#endif
