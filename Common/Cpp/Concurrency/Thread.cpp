/*  Thread
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//
//  Workaround Qt 6.9 thread-adoption bug on Windows.
//      https://github.com/PokemonAutomation/Arduino-Source/issues/570
//      https://bugreports.qt.io/browse/QTBUG-131892
//

#ifndef QT_CORE_LIB
#include "ThreadBackends/Thread_StdThread.tpp"

#else

#include <QtGlobal>

#if _WIN32 && (QT_VERSION_MAJOR * 1000000 + QT_VERSION_MINOR * 1000 + QT_VERSION_PATCH) > 6008003
#include "ThreadBackends/Thread_StdThreadDetach.tpp"
#else
#include "ThreadBackends/Thread_StdThread.tpp"
#endif

#endif
